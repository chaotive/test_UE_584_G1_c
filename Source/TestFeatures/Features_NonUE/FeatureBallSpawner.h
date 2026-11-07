#pragma once

#include "CoreMinimal.h"

class AFeatureBall;
class UWorld;

/** Reuses inactive training balls and grows the pool when all balls are active. */
class TESTFEATURES_API FFeatureBallSpawner
{
public:

	DECLARE_EVENT_OneParam(FFeatureBallSpawner, FBallCreatedEvent, AFeatureBall&);

	// Announces new pool entries so listeners can subscribe once before their first activation.
	// The native multicast event passes the new actor by reference; only FFeatureBallSpawner can broadcast it.
	FBallCreatedEvent OnBallCreated;

	// Creates inactive pool entries until the requested pool size is available.
	// World is a mutable reference, Count is passed by value, and the world owns the created actors.
	void Prewarm(UWorld& World, int32 Count);

	// Activates one pooled ball or creates a new ball when none are available.
	// World is a mutable reference, Location is read-only, and the returned raw pointer refers to a world-owned actor.
	AFeatureBall* Spawn(UWorld& World, const FVector& Location);

	// Spawns the requested number of balls at random positions within the bounds by calling Spawn.
	// World is a mutable reference, bounds are read-only references, and Count is passed by value; actors remain world-owned.
	void SpawnMultiple(UWorld& World, int32 Count, const FVector& MinLocation, const FVector& MaxLocation);

	// Provides access to all pool entries in creation order, including inactive balls.
	// Returns a const TArray reference to avoid copying; TWeakObjectPtr observes world-owned actors without keeping them alive.
	const TArray<TWeakObjectPtr<AFeatureBall>>& GetSpawnedBalls() const;

private:

	// Finds an inactive ball available for reuse, or returns null if the pool has none.
	// Returns a non-owning raw actor pointer from the weak-pointer list without transferring world ownership.
	AFeatureBall* FindAvailableBall() const;

	// Creates and records a new inactive ball with a name matching its pool index.
	// World is a mutable reference, Location is read-only, and the raw return pointer refers to a world-owned actor.
	AFeatureBall* CreateBall(UWorld& World, const FVector& Location);

	// Keeps active and inactive balls in stable creation order so reuse preserves their names and pool indices.
	// TArray grows dynamically; TWeakObjectPtr tracks world-owned actors without keeping them alive and becomes invalid when they are destroyed.
	TArray<TWeakObjectPtr<AFeatureBall>> SpawnedBalls;
};
