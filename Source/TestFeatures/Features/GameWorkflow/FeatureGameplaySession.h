#pragma once

#include "CoreMinimal.h"
#include "Features_NonUE/FeatureBallSpawner.h"
#include "Subsystems/WorldSubsystem.h"
#include "FeatureGameplaySession.generated.h"

class AFeatureGameState;
class AFeatureBall;
class AActor;
class UFeatureGameStateController;
class UFeatureSaveGameSubsystem;
struct FFeatureGameplaySessionRecord;

/** Owns gameplay-session workflows and publishes lifecycle events to observers. */
UCLASS()
class TESTFEATURES_API UFeatureGameplaySession : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	DECLARE_EVENT(UFeatureGameplaySession, FSessionEvent);
	DECLARE_EVENT_OneParam(UFeatureGameplaySession, FSessionEndedEvent, bool);

	// Announces that gameplay can provision the local player and HUD.
	// Native multicast event stores weak UObject bindings; no pointer or reference arguments.
	FSessionEvent OnStarted;

	// Announces that the paused session has returned to gameplay.
	// Native multicast event stores weak UObject bindings; no pointer or reference arguments.
	FSessionEvent OnResumed;

	// Announces that a gameplay pause request succeeded.
	// Native multicast event stores weak UObject bindings; no pointer or reference arguments.
	FSessionEvent OnPaused;

	// Announces the gameplay result so observers can present the end screen.
	// Native multicast event passes the victory bool by value and stores weak UObject bindings.
	FSessionEndedEvent OnEnded;

	// Connects gameplay events and configures the level-one victory and defeat limits.
	// State is borrowed; limits and per-level adjustments are passed by value.
	void BindGameState(AFeatureGameState& State, int32 ContactsToWin, int32 BouncesToLose, int32 ContactsIncrease, int32 BouncesDecrease);

	// Removes this subsystem's observation when its world shuts down.
	// No pointer or reference parameters or return value; releases weak UObject subscriptions.
	virtual void Deinitialize() override;

	// Starts the player workflow, initializes progress, and creates the enclosure and ball volley.
	// Configuration values are passed by value; both FBox references borrow world-space bounds.
	void OnGameplaySessionStart(bool bContinue, int32 InitialLevel, int32 BallCount, float BallSize, float BallSpeedMultiplier, int32 ReboundLimit, const FBox& SpawnBounds, const FBox& LevelBounds);

	// Unpauses the active session and announces its return to gameplay.
	// No pointer or reference parameters or return value; updates the observed state's world.
	void OnGameplaySessionResume();

	// Pauses gameplay and announces the pause workflow.
	// No pointer or reference parameters or return value; updates the observed state's world.
	void OnGameplaySessionPaused();

	// Ends gameplay, records the outcome, and announces victory or defeat to observers.
	// bVictory is passed by value without pointer or reference ownership.
	void OnGameplaySessionEnded(bool bVictory);

	// Reads the current session's player-contact victory threshold.
	// Returns int32 by value without pointers or references.
	int32 GetPlayerContactsToWin() const;

	// Reads the current session's non-player bounce defeat threshold.
	// Returns int32 by value without pointers or references.
	int32 GetSessionBouncesToLose() const;

private:
	enum class ESessionState : uint8 { Ready, Playing, Paused, Ended };

	// Tracks which lifecycle requests the session can accept.
	// Stores the private enum by value without pointer or reference ownership.
	ESessionState SessionState = ESessionState::Ready;

	// Sets the level-one player contact limit, increased by the configured amount for each subsequent level.
	// int32 stores the configured positive threshold by value without object ownership.
	int32 PlayerContactsToWin = 10;

	// Sets the level-one bounce limit, decreased per level with a minimum of one.
	// int32 stores the configured positive threshold by value without object ownership.
	int32 SessionBouncesToLose = 50;

	// Sets the per-level increase in required player contacts.
	// int32 stores the increment by value.
	int32 ContactsIncreasePerLevel = 1;

	// Sets the per-level decrease in allowed non-player bounces.
	// int32 stores the decrement by value.
	int32 BouncesDecreasePerLevel = 1;

	// Stores the uniform scale for newly created pooled balls.
	// float stores the scale by value.
	float BallSize = 1.0f;

	// Stores the launch speed multiplier for newly created pooled balls.
	// float stores the multiplier by value.
	float BallSpeedMultiplier = 1.0f;

	// Stores the rebound limit for newly created pooled balls.
	// int32 stores the positive limit by value.
	int32 BallReboundLimit = 5;

	// Ends the current session once it reaches its victory or defeat limit.
	// Stats borrows a read-only state-owned session record without transferring ownership.
	void EvaluateSessionLimits(const FFeatureGameplaySessionRecord& Stats);

	// Rechecks deferred statistics once persistence releases its gameplay pause.
	// Status is a borrowed string reference and bBusy is passed by value without ownership.
	void HandlePersistenceStatusChanged(const FString& Status, bool bBusy);

	// Subscribes the session to player contacts for each ball created by the pool.
	// Ball is a borrowed world-owned actor reference; the event holds a weak UObject binding.
	void ObserveBall(AFeatureBall& Ball);

	// Retires balls that contact the player.
	// Ball and OtherActor are borrowed world-owned references without ownership transfer.
	void HandleBallContact(AFeatureBall& Ball, AActor& OtherActor);

	// Schedules a replacement after any ball retires during gameplay.
	// Ball is a borrowed world-owned actor reference without ownership transfer.
	void HandleBallRetired(AFeatureBall& Ball);

	// Launches a replacement from the session's configured spawn bounds on the next tick.
	// No pointer or reference parameters or return value; the world owns the spawned actor.
	void SpawnReplacementBall();

	// Retains the spawn region for replacements throughout the gameplay session.
	// FBox stores the bounds by value without pointer ownership.
	FBox BallSpawnBounds;

	// Tracks retired-ball replacements until they can launch during gameplay.
	// int32 stores the pending count by value without pointer ownership.
	int32 PendingBallReplacements = 0;

	// Observes the world's shared bounce state throughout the session.
	// A weak UObject pointer observes the world-owned actor without retaining its lifetime.
	TWeakObjectPtr<AFeatureGameState> GameState;

	// Observes persistence completion so paused I/O cannot conceal a reached score limit.
	// The weak UObject pointer observes the game-instance-owned subsystem without retaining it.
	TWeakObjectPtr<UFeatureSaveGameSubsystem> SaveSubsystem;

	// Translates ball bounce events into shared state changes.
	// UPROPERTY tracks a strong TObjectPtr to the controller owned by this session.
	UPROPERTY()
	TObjectPtr<UFeatureGameStateController> GameStateController;

	// Creates balls and retains the pool's actor observations.
	// The non-UObject spawner is stored by value and owns its list of weak actor pointers.
	FFeatureBallSpawner BallSpawner;

	// Reports whether persistence currently owns the world's pause state.
	// Returns bool by value with no pointer or reference parameters.
	bool IsPersistenceBusy() const;

	// Reports whether gameplay can accept a pause or end request.
	// Returns bool by value with no pointer or reference parameters.
	bool CanLeaveGameplay() const;
};
