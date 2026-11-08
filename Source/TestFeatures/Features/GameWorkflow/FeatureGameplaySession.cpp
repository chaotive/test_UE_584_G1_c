#include "FeatureGameplaySession.h"
#include "Features/Gameplay/FeatureGameState.h"
#include "Features/Gameplay/FeatureBall.h"
#include "Features/Gameplay/FeatureGameStateController.h"
#include "Features/Gameplay/FeatureLevelBounds.h"
#include "FeatureSaveGameSubsystem.h"
#include "FeatureProgressPersistence.h"
#include "Features_GameLogic/FeatureLevelRules.h"
#include "Features_GameLogic/FeatureBallRules.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "TestFeatures.h"

void UFeatureGameplaySession::BindGameState(AFeatureGameState& State, int32 ContactsToWin, int32 BouncesToLose, int32 ContactsIncrease, int32 BouncesDecrease, int32 MinimumBouncesToLose)
{
	GameState = &State;
	SaveSubsystem = State.GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>();
	PlayerContactsToWin = ContactsToWin;
	SessionBouncesToLose = BouncesToLose;
	MinimumSessionBouncesToLose = MinimumBouncesToLose;
	ContactsIncreasePerLevel = ContactsIncrease;
	BouncesDecreasePerLevel = BouncesDecrease;
	State.OnSessionStatsChanged.AddUObject(this, &UFeatureGameplaySession::EvaluateSessionLimits);
	SaveSubsystem->OnStatusChanged.AddUObject(this, &UFeatureGameplaySession::HandlePersistenceStatusChanged);
	GameStateController = NewObject<UFeatureGameStateController>(this);
	GameStateController->Initialize(State);
	BallSpawner.OnBallCreated.AddUObject(GameStateController.Get(), &UFeatureGameStateController::ObserveBall);
	BallSpawner.OnBallCreated.AddUObject(this, &UFeatureGameplaySession::ObserveBall);
}

void UFeatureGameplaySession::Deinitialize()
{
	if (AFeatureGameState* State = GameState.Get())
	{
		State->OnSessionStatsChanged.RemoveAll(this);
	}
	if (UFeatureSaveGameSubsystem* Saves = SaveSubsystem.Get())
	{
		Saves->OnStatusChanged.RemoveAll(this);
	}
	Super::Deinitialize();
}

void UFeatureGameplaySession::EvaluateSessionLimits(const FFeatureGameplaySessionRecord& Stats)
{
	if (!CanLeaveGameplay())
	{
		return;
	}

	const EFeatureGameplaySessionResult Result = GetWorld()->GetSubsystem<UFeatureLevelRules>()->SessionOutcome(
		Stats.PlayerBallCollisions, Stats.TotalBounces, GetPlayerContactsToWin(), GetSessionBouncesToLose());
	if (Result != EFeatureGameplaySessionResult::InProgress)
	{
		OnGameplaySessionEnded(Result == EFeatureGameplaySessionResult::Victory);
	}
}

void UFeatureGameplaySession::HandlePersistenceStatusChanged(const FString&, bool bBusy)
{
	if (!bBusy && SessionState == ESessionState::Playing)
	{
		EvaluateSessionLimits(GameState->GetGameplaySessions().Last());
	}
}

void UFeatureGameplaySession::ObserveBall(AFeatureBall& Ball)
{
	Ball.Configure(BallSize, BallSpeedMultiplier, BallReboundLimit);
	Ball.OnContact.AddUObject(this, &UFeatureGameplaySession::HandleBallContact);
	Ball.OnRetired.AddUObject(this, &UFeatureGameplaySession::HandleBallRetired);
}

void UFeatureGameplaySession::HandleBallContact(AFeatureBall&, AActor& OtherActor)
{
	APawn* Pawn = Cast<APawn>(&OtherActor);
	if (Pawn && Pawn->IsPlayerControlled())
	{
		OnPlayerBallContact.Broadcast(*Pawn);
	}
}

void UFeatureGameplaySession::HandleBallRetired(AFeatureBall& Ball)
{
	if (CanReplaceBalls())
	{
		++PendingBallReplacements;
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UFeatureGameplaySession::SpawnReplacementBall);
	}
}

void UFeatureGameplaySession::SpawnReplacementBall()
{
	if (CanReplaceBalls() && PendingBallReplacements > 0)
	{
		--PendingBallReplacements;
		BallSpawner.Spawn(*GetWorld(), FMath::RandPointInBox(BallSpawnBounds));
	}
}

void UFeatureGameplaySession::OnGameplaySessionStart(bool bContinue, int32 InitialLevel, int32 BallCount, float InBallSize, float InBallSpeedMultiplier, int32 ReboundLimit, const FBox& SpawnBounds, const FBox& LevelBounds)
{
	if (SessionState != ESessionState::Ready || IsPersistenceBusy())
	{
		return;
	}

	SessionState = ESessionState::Playing;
	BallSpawnBounds = SpawnBounds;
	BallSize = InBallSize;
	BallSpeedMultiplier = InBallSpeedMultiplier;
	BallReboundLimit = ReboundLimit;
	AFeatureGameState& State = *GameState.Get();
	UWorld& World = *State.GetWorld();
	AFeatureLevelBounds* Bounds = World.SpawnActor<AFeatureLevelBounds>(LevelBounds.GetCenter(), FRotator::ZeroRotator);
	Bounds->SetHalfExtent(LevelBounds.GetExtent());
	OnStarted.Broadcast();
	State.FindComponentByClass<UFeatureProgressPersistence>()->StartSession(bContinue, InitialLevel);
	BallSpawner.Prewarm(World, BallCount);
	BallSpawner.SpawnMultiple(World, BallCount, SpawnBounds.Min, SpawnBounds.Max);
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameplaySession: Started with bounds center %s, half-extent %s"),
		*LevelBounds.GetCenter().ToString(), *LevelBounds.GetExtent().ToString());
}

void UFeatureGameplaySession::OnGameplaySessionResume()
{
	if (SessionState == ESessionState::Paused && !IsPersistenceBusy()
		&& UGameplayStatics::SetGamePaused(GameState.Get(), false))
	{
		SessionState = ESessionState::Playing;
		OnResumed.Broadcast();
		EvaluateSessionLimits(GameState->GetGameplaySessions().Last());
		while (CanReplaceBalls() && PendingBallReplacements > 0)
		{
			SpawnReplacementBall();
		}
	}
}

void UFeatureGameplaySession::OnGameplaySessionPaused()
{
	if (CanLeaveGameplay() && UGameplayStatics::SetGamePaused(GameState.Get(), true))
	{
		SessionState = ESessionState::Paused;
		OnPaused.Broadcast();
	}
}

void UFeatureGameplaySession::OnGameplaySessionEnded(bool bVictory)
{
	if (CanLeaveGameplay() && UGameplayStatics::SetGamePaused(GameState.Get(), true))
	{
		SessionState = ESessionState::Ended;
		PendingBallReplacements = 0;
		GameState->RecordSessionResult(bVictory);
		OnEnded.Broadcast(bVictory);
		GameState->FindComponentByClass<UFeatureProgressPersistence>()->SaveSession();
		UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameplaySession: Ended (%s)"), bVictory ? TEXT("Victory") : TEXT("Defeat"));
	}
}

bool UFeatureGameplaySession::IsPersistenceBusy() const
{
	return SaveSubsystem->IsBusy();
}

int32 UFeatureGameplaySession::GetPlayerContactsToWin() const
{
	return GetWorld()->GetSubsystem<UFeatureLevelRules>()->PlayerContactsToWin(
		GameState->GetCurrentLevel(), PlayerContactsToWin, ContactsIncreasePerLevel);
}

int32 UFeatureGameplaySession::GetSessionBouncesToLose() const
{
	return GetWorld()->GetSubsystem<UFeatureLevelRules>()->SessionBouncesToLose(
		GameState->GetCurrentLevel(), SessionBouncesToLose, BouncesDecreasePerLevel, MinimumSessionBouncesToLose);
}

bool UFeatureGameplaySession::CanLeaveGameplay() const
{
	return SessionState == ESessionState::Playing && !IsPersistenceBusy();
}

bool UFeatureGameplaySession::CanReplaceBalls() const
{
	return GetWorld()->GetSubsystem<UFeatureBallRules>()->ShouldReplaceBall(SessionState == ESessionState::Playing);
}
