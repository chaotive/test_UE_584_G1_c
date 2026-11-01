#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "TrainingBall.generated.h"

class UPhysicalMaterial;
class UPrimitiveComponent;

/** A sphere that launches along a random physics trajectory when play begins. */
UCLASS()
class ATrainingBall : public AStaticMeshActor
{
	GENERATED_BODY()

public:

	// Creates a training ball with its sphere mesh and physics configured.
	// No pointer or reference parameters; Unreal creates and owns this actor instance.
	ATrainingBall();

protected:

	// Logs play startup and launches the ball along a random trajectory.
	// No pointer or reference parameters or return value; operates on this actor instance.
	virtual void BeginPlay() override;

	// Logs why the ball stopped playing and runs the inherited cleanup.
	// EndPlayReason is passed by value; no pointer or reference ownership is transferred.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// Assigns the sphere mesh and enables movable physics with gravity and collision.
	// No pointer or reference parameters or return value; modifies the mesh component owned by this actor.
	void ConfigureBall();

	// Gives the ball a bouncy surface and enables collision notifications.
	// No pointer or reference parameters or return value; modifies this actor's mesh and owned material.
	void ConfigureBouncing();

	// Applies a random upward launch velocity and logs it.
	// No pointer or reference parameters or return value; modifies the mesh component owned by this actor.
	void StartRandomMovement();

	// Counts collision impulses as bounces and destroys the ball on its second bounce.
	// Component and actor pointers are non-owning event inputs, NormalImpulse is by value, and Hit is a read-only reference.
	UFUNCTION()
	void OnBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse, const FHitResult& Hit);

	// Tracks how many times this ball has bounced since it was spawned.
	// int32 stores a per-instance counter by value without pointer ownership or a wrapper.
	int32 BounceCount = 0;

	// Supplies the restitution that makes the sphere rebound on contact.
	// UPROPERTY tracks the actor-owned material for garbage collection; TObjectPtr is a strong Unreal object pointer.
	UPROPERTY()
	TObjectPtr<UPhysicalMaterial> BounceMaterial;
};
