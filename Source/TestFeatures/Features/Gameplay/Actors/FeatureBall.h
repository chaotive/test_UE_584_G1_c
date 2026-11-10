#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "FeatureBall.generated.h"

class UPhysicalMaterial;
class UPrimitiveComponent;
class UStaticMeshComponent;

/** A reusable physics sphere that returns to its pool after its configured bounce limit. */
UCLASS()
class TESTFEATURES_API AFeatureBall : public AStaticMeshActor
{
	GENERATED_BODY()

public:

	DECLARE_EVENT_OneParam(AFeatureBall, FBouncedEvent, AFeatureBall&);
	DECLARE_EVENT_TwoParams(AFeatureBall, FContactEvent, AFeatureBall&, AActor&);
	DECLARE_EVENT_OneParam(AFeatureBall, FRetiredEvent, AFeatureBall&);

	// Announces each counted bounce so independent listeners can react to it.
	// The native multicast event passes this actor by reference; only AFeatureBall can broadcast it.
	FBouncedEvent OnBounced;

	// Announces blocking contacts while the ball is active, including swept character contacts.
	// The native event borrows references to this ball and the world-owned other actor.
	FContactEvent OnContact;

	// Announces when an active ball returns to the pool.
	// The native event borrows a reference to this world-owned actor.
	FRetiredEvent OnRetired;

	// Creates a training ball with its sphere mesh and physics configured.
	// No pointer or reference parameters; Unreal creates and owns this actor instance.
	AFeatureBall();

	// Configures this pooled ball's size, launch speed, and retirement limit.
	void Configure(float Size, float SpeedMultiplier, int32 ReboundLimit);

	// Resets and launches this ball for a new use at the requested position.
	// Location is a read-only FVector reference; no pointer ownership is transferred.
	void Activate(const FVector& Location);

	// Stops and hides this ball so the ball manager can reuse it later.
	// No pointer or reference parameters or return value; modifies this world-owned actor.
	void Deactivate();

	// Reports whether this ball is currently in use rather than available in the pool.
	// Returns bool by value without pointers or references; const keeps this query read-only.
	bool IsActive() const;

protected:

	// Logs play startup, applies the bounce material, and connects collision handling.
	// No pointer or reference parameters or return value; operates on this actor instance.
	virtual void BeginPlay() override;

	// Logs why the ball stopped playing and runs the inherited cleanup.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Allows the replacement smoke check to deliver native collision callbacks.
	// The test class retains no actor, component, or script-runtime ownership.
	friend class FFeatureSmokeSpec;

	// Assigns the sphere mesh and prepares an inactive ball for later activation.
	// No pointer or reference parameters or return value; modifies the mesh component owned by this actor.
	void ConfigureBall();

	// Creates the bounce material and enables collision notifications.
	// No pointer or reference parameters or return value; modifies this actor's mesh and owned material.
	void ConfigureBouncing();

	// Applies a random upward launch velocity and logs it.
	// No pointer or reference parameters or return value; modifies the mesh component owned by this actor.
	void StartRandomMovement();

	// Reports contacts and applies the script's bounce-count and retirement effects.
	// Component and actor pointers are non-owning event inputs, NormalImpulse is by value, and Hit is a read-only reference.
	UFUNCTION()
	void OnBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse, const FHitResult& Hit);

	// Caches the inherited sphere mesh component for setup, movement, collision, and pooling.
	// UPROPERTY tracks this strong TObjectPtr through Unreal object duplication and garbage collection; the actor owns the component.
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> BallMesh;

	// Scales the initial random launch speed for each activation.
	float LaunchSpeedMultiplier = 1.0f;

	// Sets the number of non-player rebounds before retirement.
	int32 ReboundsBeforeRetirement = 5;

	// Tracks how many times this ball has bounced during its current activation.
	int32 BounceCount = 0;

	// Distinguishes active balls from inactive balls available for reuse.
	bool bActive = false;

	// Supplies the restitution that makes the sphere rebound on contact.
	// UPROPERTY tracks the actor-owned material for garbage collection; TObjectPtr is a strong Unreal object pointer.
	UPROPERTY()
	TObjectPtr<UPhysicalMaterial> BounceMaterial;
};
