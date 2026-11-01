#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "TrainingBall.generated.h"

class UPhysicalMaterial;
class UPrimitiveComponent;

/** A reusable physics sphere that returns to its pool after two bounces. */
UCLASS()
class ATrainingBall : public AStaticMeshActor
{
	GENERATED_BODY()

public:

	// Creates a training ball with its sphere mesh and physics configured.
	// No pointer or reference parameters; Unreal creates and owns this actor instance.
	ATrainingBall();

	// Resets and launches this ball for a new use at the requested position.
	// Location is a read-only FVector reference; no pointer ownership is transferred.
	void Activate(const FVector& Location);

	// Stops and hides this ball so the spawner can reuse it later.
	// No pointer or reference parameters or return value; modifies this world-owned actor.
	void Deactivate();

	// Reports whether this ball is currently in use rather than available in the pool.
	// Returns bool by value without pointers or references; const keeps this query read-only.
	bool IsActive() const;

protected:

	// Logs play startup and connects collision handling once for this actor's lifetime.
	// No pointer or reference parameters or return value; operates on this actor instance.
	virtual void BeginPlay() override;

	// Logs why the ball stopped playing and runs the inherited cleanup.
	// EndPlayReason is passed by value; no pointer or reference ownership is transferred.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// Assigns the sphere mesh and prepares an inactive ball for later activation.
	// No pointer or reference parameters or return value; modifies the mesh component owned by this actor.
	void ConfigureBall();

	// Gives the ball a bouncy surface and enables collision notifications.
	// No pointer or reference parameters or return value; modifies this actor's mesh and owned material.
	void ConfigureBouncing();

	// Applies a random upward launch velocity and logs it.
	// No pointer or reference parameters or return value; modifies the mesh component owned by this actor.
	void StartRandomMovement();

	// Counts collision impulses as bounces and returns the ball to the pool on its second bounce.
	// Component and actor pointers are non-owning event inputs, NormalImpulse is by value, and Hit is a read-only reference.
	UFUNCTION()
	void OnBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse, const FHitResult& Hit);

	// Tracks how many times this ball has bounced during its current activation.
	// int32 stores a per-instance counter by value without pointer ownership or a wrapper.
	int32 BounceCount = 0;

	// Distinguishes active balls from inactive balls available for reuse.
	// bool stores per-instance state by value without pointer ownership or a wrapper.
	bool bActive = false;

	// Supplies the restitution that makes the sphere rebound on contact.
	// UPROPERTY tracks the actor-owned material for garbage collection; TObjectPtr is a strong Unreal object pointer.
	UPROPERTY()
	TObjectPtr<UPhysicalMaterial> BounceMaterial;
};
