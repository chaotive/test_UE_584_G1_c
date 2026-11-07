#include "FeatureGameplaySession.h"
#include "FeatureGameState.h"
#include "FeatureGameStateController.h"
#include "FeatureLevelBounds.h"
#include "FeatureSaveGameSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TestFeatures.h"

void UFeatureGameplaySession::BindGameState(AFeatureGameState& State)
{
	GameState = &State;
	GameStateController = NewObject<UFeatureGameStateController>(this);
	GameStateController->Initialize(State);
	BallSpawner.OnBallCreated.AddUObject(GameStateController.Get(), &UFeatureGameStateController::ObserveBall);
}

void UFeatureGameplaySession::OnGameplaySessionStart(bool bContinue, int32 BallCount, const FBox& SpawnBounds, const FBox& LevelBounds)
{
	if (SessionState != ESessionState::Ready || IsPersistenceBusy())
	{
		return;
	}

	SessionState = ESessionState::Playing;
	AFeatureGameState& State = *GameState.Get();
	UWorld& World = *State.GetWorld();
	AFeatureLevelBounds* Bounds = World.SpawnActor<AFeatureLevelBounds>(LevelBounds.GetCenter(), FRotator::ZeroRotator);
	Bounds->SetHalfExtent(LevelBounds.GetExtent());
	OnStarted.Broadcast();
	State.StartSession(bContinue);
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
		OnEnded.Broadcast(bVictory);
	}
}

bool UFeatureGameplaySession::IsPersistenceBusy() const
{
	return GameState->GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>()->IsBusy();
}

bool UFeatureGameplaySession::CanLeaveGameplay() const
{
	return SessionState == ESessionState::Playing && !IsPersistenceBusy();
}
