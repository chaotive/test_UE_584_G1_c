#include "FeatureGameState.h"
#include "Features_GameLogic/FeatureLevelRules.h"
#include "Engine/World.h"
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
		const int32 NextLevel = GetWorld()->GetSubsystem<UFeatureLevelRules>()->NextLevel(
			Progress.CurrentLevel, Progress.Sessions.Last().Result);
		if (NextLevel != Progress.CurrentLevel)
		{
			Progress.CurrentLevel = NextLevel;
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

void AFeatureGameState::InitializeSession(FFeatureGameProgress& Snapshot)
{
	Snapshot.Sessions.AddDefaulted_GetRef().StartedAtUtc = FDateTime::UtcNow();
	Progress = Snapshot;
	bSessionInitialized = true;
	OnTotalBouncesChanged.Broadcast(Progress.TotalBounces);
	OnSessionStatsChanged.Broadcast(Progress.Sessions.Last());
	OnCurrentLevelChanged.Broadcast(Progress.CurrentLevel);
}

const FFeatureGameProgress& AFeatureGameState::GetProgress() const
{
	return Progress;
}

bool AFeatureGameState::IsSessionInitialized() const
{
	return bSessionInitialized;
}

void AFeatureGameState::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameState [%s]: BeginPlay (Total bounces: %d)"), *GetName(), Progress.TotalBounces);
}

void AFeatureGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameState [%s]: EndPlay (Total bounces: %d)"), *GetName(), Progress.TotalBounces);
	Super::EndPlay(EndPlayReason);
}
