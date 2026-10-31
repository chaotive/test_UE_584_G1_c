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

	// No pointer or reference parameters; Unreal creates and owns this actor instance.
	ATrainingBall();

protected:

	// No pointer or reference parameters; initializes this existing actor for play.
	virtual void BeginPlay() override;
	// EndPlayReason is passed by value; no pointer or reference ownership is transferred.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// No pointer or reference parameters; configures the mesh component owned by this actor.
	void ConfigureBall();
	// No pointer or reference parameters; changes the velocity of this actor's mesh component.
	void StartRandomMovement();
};
