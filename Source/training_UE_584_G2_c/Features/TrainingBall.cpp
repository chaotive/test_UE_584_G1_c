#include "TrainingBall.h"
#include "training_UE_584_G2_c.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ATrainingBall::ATrainingBall()
{
	ConfigureBall();
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBall [%s]: Constructor (base setup complete)"), *GetName());
}

void ATrainingBall::ConfigureBall()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UStaticMeshComponent* BallMesh = GetStaticMeshComponent();
	BallMesh->SetMobility(EComponentMobility::Movable);
	BallMesh->SetStaticMesh(SphereMesh.Object);
	BallMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	BallMesh->SetSimulatePhysics(true);
	BallMesh->SetEnableGravity(true);
}

void ATrainingBall::BeginPlay()
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBall [%s]: BeginPlay"), *GetName());
	Super::BeginPlay();
	StartRandomMovement();
}

void ATrainingBall::StartRandomMovement()
{
	// Launch upward with a random spread; gravity and collisions shape the trajectory.
	const FVector Direction = FMath::VRandCone(FVector::UpVector, FMath::DegreesToRadians(60.0f));
	const float Speed = FMath::FRandRange(500.0f, 900.0f);
	const FVector Velocity = Direction * Speed;
	GetStaticMeshComponent()->SetPhysicsLinearVelocity(Velocity);

	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBall [%s]: Random movement started (Velocity: %s cm/s)"),
		*GetName(), *Velocity.ToString());
}

void ATrainingBall::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBall [%s]: EndPlay (Reason: %s)"),
		*GetName(), *UEnum::GetValueAsString(EndPlayReason));
	Super::EndPlay(EndPlayReason);
}
