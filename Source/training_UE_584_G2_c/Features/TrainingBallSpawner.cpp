#include "TrainingBallSpawner.h"
#include "TrainingBall.h"
#include "training_UE_584_G2_c.h"
#include "Engine/World.h"

void FTrainingBallSpawner::SpawnMultiple(UWorld& World, int32 Count, const FVector& MinLocation, const FVector& MaxLocation)
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBallSpawner: Spawning %d balls (Min: %s, Max: %s)"),
		Count, *MinLocation.ToString(), *MaxLocation.ToString());

	const FBox SpawnBounds(MinLocation, MaxLocation);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FVector Location = FMath::RandPointInBox(SpawnBounds);
		Spawn(World, Location);
	}
}

ATrainingBall* FTrainingBallSpawner::Spawn(UWorld& World, const FVector& Location)
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBallSpawner: Spawn started (Location: %s)"), *Location.ToString());

	FActorSpawnParameters SpawnParameters;
	const FString BallName = FString::Printf(TEXT("Training_Ball_%d"), SpawnedBalls.Num());
	SpawnParameters.Name = FName(*BallName);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATrainingBall* Ball = World.SpawnActor<ATrainingBall>(Location, FRotator::ZeroRotator, SpawnParameters);
	Ball->SetActorLabel(BallName);
	SpawnedBalls.Add(Ball);

	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBallSpawner [%s]: Spawn complete"), *Ball->GetName());
	return Ball;
}

const TArray<TWeakObjectPtr<ATrainingBall>>& FTrainingBallSpawner::GetSpawnedBalls() const
{
	return SpawnedBalls;
}
