#include "TrainingBallSpawner.h"
#include "training_UE_584_G2_c.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"

AStaticMeshActor* FTrainingBallSpawner::Spawn(UWorld& World, const FVector& Location)
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBallSpawner: Spawn started (Location: %s)"), *Location.ToString());

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = TEXT("Training_Ball");
	AStaticMeshActor* Ball = World.SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator, SpawnParameters);
	Ball->SetActorLabel(TEXT("Training_Ball"));
	UStaticMeshComponent* BallMesh = Ball->GetStaticMeshComponent();
	BallMesh->SetMobility(EComponentMobility::Movable);
	BallMesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")));
	BallMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBallSpawner [%s]: Spawn complete (sphere mesh and collision configured)"), *Ball->GetName());
	return Ball;
}
