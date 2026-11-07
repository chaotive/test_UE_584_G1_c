#include "FeatureGameState.h"
#include "Features/GameWorkflow/FeatureSaveGameSubsystem.h"
#include "Features_NonUE/FeatureSaveGameStorage.h"
#include "Engine/GameInstance.h"
#include "TestFeatures.h"

void AFeatureGameState::RecordBounce()
{
	if (!bSessionInitialized || Progress.Sessions.Last().Result != EFeatureGameplaySessionResult::InProgress)
	{
		return;
	}
	++Progress.TotalBounces;
	++Progress.Sessions.Last().TotalBounces;
	OnTotalBouncesChanged.Broadcast(Progress.TotalBounces);
	OnSessionStatsChanged.Broadcast(Progress.Sessions.Last());
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameState [%s]: Total bounces: %d"), *GetName(), Progress.TotalBounces);
}

void AFeatureGameState::RecordPlayerBallCollision()
{
	if (!bSessionInitialized || Progress.Sessions.Last().Result != EFeatureGameplaySessionResult::InProgress)
	{
		return;
	}
	++Progress.Sessions.Last().PlayerBallCollisions;
	OnSessionStatsChanged.Broadcast(Progress.Sessions.Last());
}

void AFeatureGameState::RecordSessionResult(bool bVictory)
{
	if (bSessionInitialized && Progress.Sessions.Last().Result == EFeatureGameplaySessionResult::InProgress)
	{
		Progress.Sessions.Last().Result = bVictory
			? EFeatureGameplaySessionResult::Victory
			: EFeatureGameplaySessionResult::Defeat;
		if (bVictory)
		{
			++Progress.CurrentLevel;
			OnCurrentLevelChanged.Broadcast(Progress.CurrentLevel);
		}
	}
}

const TArray<FFeatureGameplaySessionRecord>& AFeatureGameState::GetGameplaySessions() const
{
	return Progress.Sessions;
}

int32 AFeatureGameState::GetCurrentLevel() const
{
	return Progress.CurrentLevel;
}

int32 AFeatureGameState::GetTotalBounces() const
{
	return Progress.TotalBounces;
}

void AFeatureGameState::InitializeProgress(FFeatureGameProgress& Snapshot)
{
	Snapshot.Sessions.AddDefaulted_GetRef().StartedAtUtc = FDateTime::UtcNow();
	Progress = Snapshot;
	bSessionInitialized = true;
	OnTotalBouncesChanged.Broadcast(Progress.TotalBounces);
	OnSessionStatsChanged.Broadcast(Progress.Sessions.Last());
	OnCurrentLevelChanged.Broadcast(Progress.CurrentLevel);
}

void AFeatureGameState::StartSession(bool bContinue)
{
	if (bContinue)
	{
		GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>()->RequestLoad();
	}
	else
	{
		FFeatureGameProgress FreshProgress;
		InitializeProgress(FreshProgress);
	}
}

void AFeatureGameState::SaveSession(bool bQuitAfter)
{
	GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>()->RequestSave(Progress, bQuitAfter);
}

void AFeatureGameState::BeginPlay()
{
	Super::BeginPlay();
	UFeatureSaveGameSubsystem* Saves = GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>();
	Saves->OnLoaded.AddUObject(this, &AFeatureGameState::InitializeProgress);
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameState [%s]: BeginPlay (Total bounces: %d)"), *GetName(), Progress.TotalBounces);
}

void AFeatureGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UFeatureSaveGameSubsystem* Saves = GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>();
	Saves->OnLoaded.RemoveAll(this);
	Saves->CancelOperation();
	// Editor Stop/forced teardown cannot be deferred for a live HUD; retain a synchronous fallback.
	if (bSessionInitialized && !Saves->HasSavedForQuit())
	{
		FFeatureSaveGameStorage::SaveProgress(Progress);
	}
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameState [%s]: EndPlay (Total bounces: %d)"), *GetName(), Progress.TotalBounces);
	Super::EndPlay(EndPlayReason);
}
