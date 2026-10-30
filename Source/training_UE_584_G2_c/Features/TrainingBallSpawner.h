#pragma once

#include "CoreMinimal.h"

class AStaticMeshActor;
class UWorld;

/** Creates and configures the ball used by the sandbox. */
class FTrainingBallSpawner
{
public:

	static AStaticMeshActor* Spawn(UWorld& World, const FVector& Location);
};
