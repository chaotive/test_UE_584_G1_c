#pragma once

#include "CoreMinimal.h"

class AStaticMeshActor;
class UWorld;

/** Creates the sandbox ball and launches it along a random physics trajectory. */
class FTrainingBallSpawner
{
public:

	// World is a mutable reference, Location is a read-only reference, and the returned raw pointer refers to a world-owned actor.
	static AStaticMeshActor* Spawn(UWorld& World, const FVector& Location);

private:

	// World is a mutable reference, Location is a read-only reference, and the returned raw pointer refers to a world-owned actor.
	static AStaticMeshActor* CreateBall(UWorld& World, const FVector& Location);
	// Ball is a mutable reference to an existing actor; no pointer is returned or ownership transferred.
	static void StartRandomMovement(AStaticMeshActor& Ball);
};
