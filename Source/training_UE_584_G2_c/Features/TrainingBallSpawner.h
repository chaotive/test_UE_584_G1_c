#pragma once

#include "CoreMinimal.h"

class ATrainingBall;
class UWorld;

/** Spawns world-owned training ball instances. */
class FTrainingBallSpawner
{
public:

	// World is a mutable reference, Location is a read-only reference, and the returned raw pointer refers to a world-owned actor.
	static ATrainingBall* Spawn(UWorld& World, const FVector& Location);
};
