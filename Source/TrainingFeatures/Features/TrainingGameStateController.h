#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TrainingGameStateController.generated.h"

class ATrainingBall;
class ATrainingGameState;

/** Translates gameplay events into updates to the shared game state. */
UCLASS()
class TRAININGFEATURES_API UTrainingGameStateController : public UObject
{
	GENERATED_BODY()

public:

	// Selects the shared game state that receives updates from gameplay events.
	// State is a mutable reference; its address is cached without transferring world ownership.
	void Initialize(ATrainingGameState& State);

	// Subscribes to a newly created ball's bounce events for its entire pooled lifetime.
	// Ball is a mutable reference; AddUObject stores a weak binding to this controller rather than owning it.
	void ObserveBall(ATrainingBall& Ball);

private:

	// Converts one ball bounce event into an increment of the shared session total.
	// Ball is a mutable reference supplied by the event; no pointer ownership is transferred.
	void HandleBallBounced(ATrainingBall& Ball);

	// Holds the game state updated by the controller's event handlers.
	// UPROPERTY tracks this strong TObjectPtr for garbage collection; the world owns the game state actor.
	UPROPERTY()
	TObjectPtr<ATrainingGameState> TrainingGameState;
};
