#include "TrainingBallSpawner.h"
#include "TrainingBall.h"
#include "training_UE_584_G2_c.h"
#include "Engine/World.h"

ATrainingBall* FTrainingBallSpawner::Spawn(UWorld& World, const FVector& Location)
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBallSpawner: Spawn started (Location: %s)"), *Location.ToString());

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = TEXT("Training_Ball");
	ATrainingBall* Ball = World.SpawnActor<ATrainingBall>(Location, FRotator::ZeroRotator, SpawnParameters);
	Ball->SetActorLabel(TEXT("Training_Ball"));

	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBallSpawner [%s]: Spawn complete"), *Ball->GetName());
	return Ball;
}
