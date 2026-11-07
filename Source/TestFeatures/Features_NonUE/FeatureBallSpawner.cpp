#include "FeatureBallSpawner.h"
#include "Features/Gameplay/FeatureBall.h"
#include "TestFeatures.h"
#include "Engine/World.h"

void FFeatureBallSpawner::SpawnMultiple(UWorld& World, int32 Count, const FVector& MinLocation, const FVector& MaxLocation)
{
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureBallSpawner: Spawning %d balls (Min: %s, Max: %s)"),
		Count, *MinLocation.ToString(), *MaxLocation.ToString());

	const FBox SpawnBounds(MinLocation, MaxLocation);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FVector Location = FMath::RandPointInBox(SpawnBounds);
		Spawn(World, Location);
	}
}

AFeatureBall* FFeatureBallSpawner::Spawn(UWorld& World, const FVector& Location)
{
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureBallSpawner: Spawn started (Location: %s)"), *Location.ToString());

	AFeatureBall* Ball = FindAvailableBall();
	if (Ball)
	{
		UE_LOG(LogTestFeatures, Log, TEXT("FeatureBallSpawner [%s]: Reusing pooled ball"), *Ball->GetName());
	}
	else
	{
		Ball = CreateBall(World, Location);
	}

	Ball->Activate(Location);
	return Ball;
}

AFeatureBall* FFeatureBallSpawner::FindAvailableBall() const
{
	for (const TWeakObjectPtr<AFeatureBall>& Entry : SpawnedBalls)
	{
		AFeatureBall* Ball = Entry.Get();
		if (Ball && !Ball->IsActive())
		{
			return Ball;
		}
	}
	return nullptr;
}

AFeatureBall* FFeatureBallSpawner::CreateBall(UWorld& World, const FVector& Location)
{
	FActorSpawnParameters SpawnParameters;
	const FString BallName = FString::Printf(TEXT("Training_Ball_%d"), SpawnedBalls.Num());
	SpawnParameters.Name = FName(*BallName);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFeatureBall* Ball = World.SpawnActor<AFeatureBall>(Location, FRotator::ZeroRotator, SpawnParameters);
#if WITH_EDITOR
	Ball->SetActorLabel(BallName);
#endif
	SpawnedBalls.Add(Ball);
	// Notify listeners once per actor creation, before Spawn activates it; pool reuse skips CreateBall.
	OnBallCreated.Broadcast(*Ball);

	UE_LOG(LogTestFeatures, Log, TEXT("FeatureBallSpawner [%s]: Pool grew to %d balls"), *Ball->GetName(), SpawnedBalls.Num());
	return Ball;
}

const TArray<TWeakObjectPtr<AFeatureBall>>& FFeatureBallSpawner::GetSpawnedBalls() const
{
	return SpawnedBalls;
}
