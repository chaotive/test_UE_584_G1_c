#include "TrainingGameStateController.h"
#include "TrainingBall.h"
#include "TrainingGameState.h"
#include "training_UE_584_G2_c.h"

void UTrainingGameStateController::Initialize(ATrainingGameState& State)
{
	// Cache the world-owned state supplied by the game mode as the destination for event updates.
	TrainingGameState = &State;
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingGameStateController: Initialized"));
}

void UTrainingGameStateController::ObserveBall(ATrainingBall& Ball)
{
	// The spawner's creation event reaches this method once, so reusing a ball adds no duplicate binding.
	// AddUObject stores a weak controller binding and ignores it if the controller is no longer valid.
	Ball.OnBounced.AddUObject(this, &UTrainingGameStateController::HandleBallBounced);
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingGameStateController: Observing %s"), *Ball.GetName());
}

void UTrainingGameStateController::HandleBallBounced(ATrainingBall& Ball)
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingGameStateController: Bounce received from %s"), *Ball.GetName());
	// Translate the event into a state update here, keeping the ball independent of the game state.
	TrainingGameState->RecordBounce();
}
