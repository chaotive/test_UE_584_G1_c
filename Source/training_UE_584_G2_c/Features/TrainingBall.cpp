#include "TrainingBall.h"
#include "training_UE_584_G2_c.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "UObject/ConstructorHelpers.h"

ATrainingBall::ATrainingBall()
{
	ConfigureBall();
	ConfigureBouncing();
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBall [%s]: Constructor (base setup complete)"), *GetName());
}

void ATrainingBall::ConfigureBall()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UStaticMeshComponent* BallMesh = GetStaticMeshComponent();
	BallMesh->SetMobility(EComponentMobility::Movable);
	BallMesh->SetStaticMesh(SphereMesh.Object);
	BallMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	BallMesh->SetSimulatePhysics(false);
	BallMesh->SetEnableGravity(true);
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
}

void ATrainingBall::BeginPlay()
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBall [%s]: BeginPlay"), *GetName());
	Super::BeginPlay();
	GetStaticMeshComponent()->OnComponentHit.AddDynamic(this, &ATrainingBall::OnBallHit);
}

void ATrainingBall::Activate(const FVector& Location)
{
	BounceCount = 0;
	bActive = true;
	SetActorLocationAndRotation(Location, FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	UStaticMeshComponent* BallMesh = GetStaticMeshComponent();
	BallMesh->SetSimulatePhysics(true);
	BallMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	StartRandomMovement();
	BallMesh->WakeAllRigidBodies();
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBall [%s]: Activated (Location: %s)"), *GetName(), *Location.ToString());
}

void ATrainingBall::Deactivate()
{
	bActive = false;
	UStaticMeshComponent* BallMesh = GetStaticMeshComponent();
	BallMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	BallMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	BallMesh->SetSimulatePhysics(false);
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBall [%s]: Returned to pool"), *GetName());
}

bool ATrainingBall::IsActive() const
{
	return bActive;
}

void ATrainingBall::ConfigureBouncing()
{
	BounceMaterial = CreateDefaultSubobject<UPhysicalMaterial>(TEXT("BounceMaterial"));
	BounceMaterial->Restitution = 0.8f;
	BounceMaterial->bOverrideRestitutionCombineMode = true;
	BounceMaterial->RestitutionCombineMode = EFrictionCombineMode::Max;

	UStaticMeshComponent* BallMesh = GetStaticMeshComponent();
	BallMesh->SetPhysMaterialOverride(BounceMaterial);
	BallMesh->SetNotifyRigidBodyCollision(true);
}

void ATrainingBall::OnBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	// Ignore queued contacts after release and contacts with no bounce impulse.
	if (!bActive || NormalImpulse.IsNearlyZero())
	{
		return;
	}

	++BounceCount;
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingBall [%s]: Bounce %d/2"), *GetName(), BounceCount);

	if (BounceCount == 2)
	{
		Deactivate();
	}
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
