#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FeatureGameStateController.generated.h"

class AFeatureBall;
class AFeatureGameState;
class AActor;

/** Translates gameplay events into updates to the shared game state. */
UCLASS()
class TESTFEATURES_API UFeatureGameStateController : public UObject
{
	GENERATED_BODY()

public:

	// Selects the shared game state that receives updates from gameplay events.
	// State is a mutable reference; its address is cached without transferring world ownership.
	void Initialize(AFeatureGameState& State);

	// Subscribes to a ball's bounce and contact events for its entire pooled lifetime.
	// Ball is a mutable reference; AddUObject stores a weak binding to this controller rather than owning it.
	void ObserveBall(AFeatureBall& Ball);

private:

	// Converts one ball bounce event into an increment of the shared session total.
	// Ball is a mutable reference supplied by the event; no pointer ownership is transferred.
	void HandleBallBounced(AFeatureBall& Ball);

	// Records ball contacts with player-controlled pawns in the active session.
	// Ball and OtherActor are borrowed world-owned references without ownership transfer.
	void HandleBallContact(AFeatureBall& Ball, AActor& OtherActor);

	// Holds the game state updated by the controller's event handlers.
	// UPROPERTY tracks this strong TObjectPtr for garbage collection; the world owns the game state actor.
	UPROPERTY()
	TObjectPtr<AFeatureGameState> FeatureGameState;
};
