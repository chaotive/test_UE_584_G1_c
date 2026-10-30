#include "TrainingBallSpawner.h"
#include "training_UE_584_G2_c.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"

AStaticMeshActor* FTrainingBallSpawner::Spawn(UWorld& World, const FVector& Location)
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBallSpawner: Spawn started (Location: %s)"), *Location.ToString());

	AStaticMeshActor* Ball = CreateBall(World, Location);
	StartRandomMovement(*Ball);

	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBallSpawner [%s]: Spawn complete"), *Ball->GetName());
	return Ball;
}

AStaticMeshActor* FTrainingBallSpawner::CreateBall(UWorld& World, const FVector& Location)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = TEXT("Training_Ball");
	AStaticMeshActor* Ball = World.SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator, SpawnParameters);
	Ball->SetActorLabel(TEXT("Training_Ball"));
	UStaticMeshComponent* BallMesh = Ball->GetStaticMeshComponent();
	BallMesh->SetMobility(EComponentMobility::Movable);
	BallMesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")));

	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBallSpawner [%s]: Base setup complete"), *Ball->GetName());
	return Ball;
}

void FTrainingBallSpawner::StartRandomMovement(AStaticMeshActor& Ball)
{
	UStaticMeshComponent* BallMesh = Ball.GetStaticMeshComponent();
	BallMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	BallMesh->SetSimulatePhysics(true);
	BallMesh->SetEnableGravity(true);

	// Launch upward with a random spread; gravity and collisions shape the trajectory.
	const FVector Direction = FMath::VRandCone(FVector::UpVector, FMath::DegreesToRadians(60.0f));
	const float Speed = FMath::FRandRange(500.0f, 900.0f);
	const FVector Velocity = Direction * Speed;
	BallMesh->SetPhysicsLinearVelocity(Velocity);

	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBallSpawner [%s]: Random movement started (Velocity: %s cm/s)"),
		*Ball.GetName(), *Velocity.ToString());
}
