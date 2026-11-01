#pragma once

#include "CoreMinimal.h"

class ATrainingBall;
class UWorld;

/** Spawns world-owned training ball instances. */
class FTrainingBallSpawner
{
public:

	// Spawns and records one training ball, naming it after its position in the list.
	// World is a mutable reference, Location is read-only, and the returned raw pointer refers to a world-owned actor.
	ATrainingBall* Spawn(UWorld& World, const FVector& Location);

	// Spawns the requested number of balls at random positions within the bounds by calling Spawn.
	// World is a mutable reference, bounds are read-only references, and Count is passed by value; actors remain world-owned.
	void SpawnMultiple(UWorld& World, int32 Count, const FVector& MinLocation, const FVector& MaxLocation);

	// Provides access to the balls recorded in creation order.
	// Returns a const TArray reference to avoid copying; TWeakObjectPtr observes world-owned actors without keeping them alive.
	const TArray<TWeakObjectPtr<ATrainingBall>>& GetSpawnedBalls() const;

private:

	// Records spawned balls in creation order so their names match their list indices.
	// TArray grows dynamically; TWeakObjectPtr tracks world-owned actors without keeping them alive and becomes invalid when they are destroyed.
	TArray<TWeakObjectPtr<ATrainingBall>> SpawnedBalls;
};
