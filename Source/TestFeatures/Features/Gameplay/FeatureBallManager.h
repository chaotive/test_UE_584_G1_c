#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FeatureBallManager.generated.h"

class AFeatureBall;
class APawn;
class AActor;

/** Maintains the world's ball pool and replacements independently of session flow. */
UCLASS()
class TESTFEATURES_API UFeatureBallManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	DECLARE_EVENT_OneParam(UFeatureBallManager, FBallCreatedEvent, AFeatureBall&);
	DECLARE_EVENT_OneParam(UFeatureBallManager, FPlayerBallContactEvent, APawn&);

	// Announces each configured pool entry before its first activation.
	// The native event borrows a world-owned actor reference and stores weak UObject bindings.
	FBallCreatedEvent OnBallCreated;

	// Publishes player contacts for independent reactions such as movement bonuses.
	// The native event borrows a world-owned pawn reference without transferring ownership.
	FPlayerBallContactEvent OnPlayerBallContact;

	// Releases timer callbacks and subscriptions when the world shuts down.
	// No pointer or reference parameters or return value; actors remain world-owned.
	virtual void Deinitialize() override;

	// Configures the pool and launches the initial ball volley.
	// Scalars are copied and SpawnBounds is borrowed read-only; the world owns all ball actors.
	void Start(int32 BallCount, float InBallSize, float InBallSpeedMultiplier, int32 ReboundLimit, const FBox& SpawnBounds);

	// Defers queued replacements while gameplay is paused.
	// No pointer or reference parameters or return value.
	void Pause();

	// Allows replacements again and consumes the deferred queue.
	// No pointer or reference parameters or return value.
	void Resume();

	// Stops replacement scheduling and discards queued work at session end.
	// No pointer or reference parameters or return value; pooled actors remain world-owned.
	void Stop();

	// Creates inactive pool entries up to the requested count.
	// Count is copied; this world's actors are observed weakly without retaining their lifetime.
	void Prewarm(int32 Count);

	// Activates a batch of pooled balls at random positions in the supplied region.
	// Count is copied and SpawnBounds is borrowed read-only; the world owns the activated actors.
	void SpawnMultiple(int32 Count, const FBox& SpawnBounds);

	// Activates an available ball or creates a new pool entry when necessary.
	// Location is borrowed read-only; the returned raw pointer observes a world-owned actor.
	AFeatureBall* Spawn(const FVector& Location);

	// Reads active and inactive pool entries in creation order.
	// The const array reference avoids copying; weak pointers observe world-owned actors.
	const TArray<TWeakObjectPtr<AFeatureBall>>& GetSpawnedBalls() const;

private:
	// Grants the existing replacement smoke check access to pool and queue observations.
	// The test class owns no runtime manager or actor references.
	friend class FFeaturePlayerBallReplacementSmokeTest;

	// Creates, configures, and observes a new pool entry before announcing it.
	// Location is borrowed read-only; the raw return pointer observes a world-owned actor.
	AFeatureBall* CreateBall(const FVector& Location);

	// Finds an inactive pool entry that can be activated again.
	// Returns a non-owning raw pointer, or null when no live inactive actor is available.
	AFeatureBall* FindAvailableBall() const;

	// Publishes contacts involving player-controlled pawns.
	// Ball and OtherActor are borrowed world-owned references without ownership transfer.
	void HandleBallContact(AFeatureBall& Ball, AActor& OtherActor);

	// Queues a next-tick replacement for a retired ball during active play.
	// Ball is a borrowed world-owned reference without ownership transfer.
	void HandleBallRetired(AFeatureBall& Ball);

	// Reuses a pooled ball from the configured spawn region when replacement is allowed.
	// No pointer or reference parameters or return value; the world owns the spawned actor.
	void SpawnReplacementBall();

	// Queries the existing scripted replacement policy with native activity state.
	// Returns bool by value without pointer or reference parameters.
	bool CanReplaceBalls() const;

	// Stores whether lifecycle coordination currently permits replacements.
	// bool is native activity state passed by value to the script.
	bool bRunning = false;

	// Retains the uniform scale configured for newly created balls.
	// float stores configuration by value without pointer ownership.
	float BallSize = 1.0f;

	// Retains the launch speed multiplier for newly created balls.
	// float stores configuration by value without pointer ownership.
	float BallSpeedMultiplier = 1.0f;

	// Retains the non-player rebound retirement threshold.
	// int32 stores configuration by value without pointer ownership.
	int32 BallReboundLimit = 5;

	// Retains the region used for initial and replacement spawns.
	// FBox stores world-space bounds by value without actor references.
	FBox BallSpawnBounds;

	// Tracks queued replacements across pause and resume.
	// int32 stores a count by value without pointer ownership.
	int32 PendingBallReplacements = 0;

	// Retains pool entries in creation order so reuse preserves actor names and indices.
	// TArray owns weak observations; the world retains ownership of active and inactive actors.
	TArray<TWeakObjectPtr<AFeatureBall>> SpawnedBalls;
};
