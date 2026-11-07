#include "FeatureGameplaySession.h"
#include "Features/Gameplay/FeatureGameState.h"
#include "Features/Gameplay/FeatureBall.h"
#include "Features/Gameplay/FeatureGameStateController.h"
#include "Features/Gameplay/FeatureLevelBounds.h"
#include "FeatureSaveGameSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "TestFeatures.h"

void UFeatureGameplaySession::BindGameState(AFeatureGameState& State, int32 ContactsToWin, int32 BouncesToLose)
{
	GameState = &State;
	SaveSubsystem = State.GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>();
	PlayerContactsToWin = ContactsToWin;
	SessionBouncesToLose = BouncesToLose;
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

	const bool bVictory = Stats.PlayerBallCollisions >= PlayerContactsToWin;
	const bool bDefeat = Stats.TotalBounces >= SessionBouncesToLose;
	if (bVictory || bDefeat)
	{
		OnGameplaySessionEnded(bVictory);
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
	Ball.OnContact.AddUObject(this, &UFeatureGameplaySession::HandleBallContact);
}

void UFeatureGameplaySession::HandleBallContact(AFeatureBall& Ball, AActor& OtherActor)
{
	const APawn* Pawn = Cast<APawn>(&OtherActor);
	if (Pawn && Pawn->IsPlayerControlled())
	{
		Ball.Deactivate();
		if (SessionState == ESessionState::Playing)
		{
			++PendingBallReplacements;
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UFeatureGameplaySession::SpawnReplacementBall);
		}
	}
}

void UFeatureGameplaySession::SpawnReplacementBall()
{
	if (SessionState == ESessionState::Playing && PendingBallReplacements > 0)
	{
		--PendingBallReplacements;
		BallSpawner.Spawn(*GetWorld(), FMath::RandPointInBox(BallSpawnBounds));
	}
}

void UFeatureGameplaySession::OnGameplaySessionStart(bool bContinue, int32 BallCount, const FBox& SpawnBounds, const FBox& LevelBounds)
{
	if (SessionState != ESessionState::Ready || IsPersistenceBusy())
	{
		return;
	}

	SessionState = ESessionState::Playing;
	BallSpawnBounds = SpawnBounds;
	AFeatureGameState& State = *GameState.Get();
	UWorld& World = *State.GetWorld();
	AFeatureLevelBounds* Bounds = World.SpawnActor<AFeatureLevelBounds>(LevelBounds.GetCenter(), FRotator::ZeroRotator);
	Bounds->SetHalfExtent(LevelBounds.GetExtent());
	OnStarted.Broadcast();
	State.StartSession(bContinue);
	BallSpawner.Prewarm(World, BallCount * 3);
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
		while (SessionState == ESessionState::Playing && PendingBallReplacements > 0)
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
		UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameplaySession: Ended (%s)"), bVictory ? TEXT("Victory") : TEXT("Defeat"));
	}
}

bool UFeatureGameplaySession::IsPersistenceBusy() const
{
	return SaveSubsystem->IsBusy();
}

int32 UFeatureGameplaySession::GetPlayerContactsToWin() const
{
	return PlayerContactsToWin;
}

int32 UFeatureGameplaySession::GetSessionBouncesToLose() const
{
	return SessionBouncesToLose;
}

bool UFeatureGameplaySession::CanLeaveGameplay() const
{
	return SessionState == ESessionState::Playing && !IsPersistenceBusy();
}
