#include "FeatureGameStateController.h"
#include "FeatureBall.h"
#include "FeatureGameState.h"
#include "GameFramework/Pawn.h"
#include "TestFeatures.h"

void UFeatureGameStateController::Initialize(AFeatureGameState& State)
{
	// Cache the world-owned state supplied by the gameplay session.
	FeatureGameState = &State;
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameStateController: Initialized"));
}

void UFeatureGameStateController::ObserveBall(AFeatureBall& Ball)
{
	// The spawner's creation event reaches this method once, so reusing a ball adds no duplicate binding.
	// AddUObject stores a weak controller binding and ignores it if the controller is no longer valid.
	Ball.OnBounced.AddUObject(this, &UFeatureGameStateController::HandleBallBounced);
	Ball.OnContact.AddUObject(this, &UFeatureGameStateController::HandleBallContact);
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameStateController: Observing %s"), *Ball.GetName());
}

void UFeatureGameStateController::HandleBallBounced(AFeatureBall& Ball)
{
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameStateController: Bounce received from %s"), *Ball.GetName());
	// Translate the event into a state update here, keeping the ball independent of the game state.
	FeatureGameState->RecordBounce();
}

void UFeatureGameStateController::HandleBallContact(AFeatureBall& Ball, AActor& OtherActor)
{
	const APawn* Pawn = Cast<APawn>(&OtherActor);
	if (Pawn && Pawn->IsPlayerControlled())
	{
		FeatureGameState->RecordPlayerBallCollision();
		UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameStateController: Player contact from %s"), *Ball.GetName());
	}
}
