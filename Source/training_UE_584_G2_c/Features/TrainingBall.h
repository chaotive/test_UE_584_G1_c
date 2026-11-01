#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "TrainingBall.generated.h"

/** A sphere that launches along a random physics trajectory when play begins. */
UCLASS()
class ATrainingBall : public AStaticMeshActor
{
	GENERATED_BODY()

public:

	// Creates a training ball with its sphere mesh and physics configured.
	// No pointer or reference parameters; Unreal creates and owns this actor instance.
	ATrainingBall();

protected:

	// Logs play startup and launches the ball along a random trajectory.
	// No pointer or reference parameters or return value; operates on this actor instance.
	virtual void BeginPlay() override;

	// Logs why the ball stopped playing and runs the inherited cleanup.
	// EndPlayReason is passed by value; no pointer or reference ownership is transferred.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// Assigns the sphere mesh and enables movable physics with gravity and collision.
	// No pointer or reference parameters or return value; modifies the mesh component owned by this actor.
	void ConfigureBall();

	// Applies a random upward launch velocity and logs it.
	// No pointer or reference parameters or return value; modifies the mesh component owned by this actor.
	void StartRandomMovement();
};
