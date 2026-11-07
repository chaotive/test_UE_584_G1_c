#include "FeatureBall.h"
#include "TestFeatures.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "UObject/ConstructorHelpers.h"

AFeatureBall::AFeatureBall()
{
	BallMesh = GetStaticMeshComponent();
	ConfigureBall();
	ConfigureBouncing();
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureBall [%s]: Constructor (base setup complete)"), *GetName());
}

void AFeatureBall::ConfigureBall()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	BallMesh->SetMobility(EComponentMobility::Movable);
	BallMesh->SetStaticMesh(SphereMesh.Object);
	BallMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	BallMesh->SetSimulatePhysics(false);
	BallMesh->SetEnableGravity(true);
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
}

void AFeatureBall::BeginPlay()
{
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureBall [%s]: BeginPlay"), *GetName());
	Super::BeginPlay();
	BallMesh->SetPhysMaterialOverride(BounceMaterial);
	BallMesh->OnComponentHit.AddDynamic(this, &AFeatureBall::OnBallHit);
}

void AFeatureBall::Activate(const FVector& Location)
{
	BounceCount = 0;
	bActive = true;
	SetActorLocationAndRotation(Location, FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	BallMesh->SetSimulatePhysics(true);
	BallMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	StartRandomMovement();
	BallMesh->WakeAllRigidBodies();
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureBall [%s]: Activated (Location: %s)"), *GetName(), *Location.ToString());
}

void AFeatureBall::Deactivate()
{
	bActive = false;
	BallMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	BallMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	BallMesh->SetSimulatePhysics(false);
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureBall [%s]: Returned to pool"), *GetName());
}

bool AFeatureBall::IsActive() const
{
	return bActive;
}

void AFeatureBall::ConfigureBouncing()
{
	BounceMaterial = CreateDefaultSubobject<UPhysicalMaterial>(TEXT("BounceMaterial"));
	BounceMaterial->Restitution = 0.8f;
	BounceMaterial->bOverrideRestitutionCombineMode = true;
	BounceMaterial->RestitutionCombineMode = EFrictionCombineMode::Max;

	BallMesh->SetNotifyRigidBodyCollision(true);
}

void AFeatureBall::OnBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	// Ignore queued contacts after release and contacts with no bounce impulse.
	if (!bActive || NormalImpulse.IsNearlyZero())
	{
		return;
	}

	++BounceCount;
	// Broadcast calls subscribed handlers immediately; the ball does not know which listeners update state.
	OnBounced.Broadcast(*this);
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureBall [%s]: Bounce %d/2"), *GetName(), BounceCount);

	if (BounceCount == 2)
	{
		Deactivate();
	}
}

void AFeatureBall::StartRandomMovement()
{
	// Launch upward with a random spread; gravity and collisions shape the trajectory.
	const FVector Direction = FMath::VRandCone(FVector::UpVector, FMath::DegreesToRadians(60.0f));
	const float Speed = FMath::FRandRange(500.0f, 900.0f);
	const FVector Velocity = Direction * Speed;
	BallMesh->SetPhysicsLinearVelocity(Velocity);

	UE_LOG(LogTestFeatures, Log, TEXT("FeatureBall [%s]: Random movement started (Velocity: %s cm/s)"),
		*GetName(), *Velocity.ToString());
}

void AFeatureBall::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureBall [%s]: EndPlay (Reason: %s)"),
		*GetName(), *UEnum::GetValueAsString(EndPlayReason));
	Super::EndPlay(EndPlayReason);
}
