#include "TrainingBallSpawner.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"

AStaticMeshActor* FTrainingBallSpawner::Spawn(UWorld& World, const FVector& Location)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = TEXT("Training_Ball");
	AStaticMeshActor* Ball = World.SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator, SpawnParameters);
	Ball->SetActorLabel(TEXT("Training_Ball"));
	UStaticMeshComponent* BallMesh = Ball->GetStaticMeshComponent();
	BallMesh->SetMobility(EComponentMobility::Movable);
	BallMesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")));
	BallMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	return Ball;
}
