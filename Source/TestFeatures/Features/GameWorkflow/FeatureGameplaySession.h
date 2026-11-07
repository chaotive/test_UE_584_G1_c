#pragma once

#include "CoreMinimal.h"
#include "Features_NonUE/FeatureBallSpawner.h"
#include "Subsystems/WorldSubsystem.h"
#include "FeatureGameplaySession.generated.h"

class AFeatureGameState;
class UFeatureGameStateController;

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

	// Connects this session's ball events to the world's shared game state.
	// State is a borrowed actor reference; this object owns the event controller.
	void BindGameState(AFeatureGameState& State);

	// Starts the player workflow, initializes progress, and creates the enclosure and ball volley.
	// bContinue and BallCount are values; both FBox references borrow world-space bounds.
	void OnGameplaySessionStart(bool bContinue, int32 BallCount, const FBox& SpawnBounds, const FBox& LevelBounds);

	// Unpauses the active session and announces its return to gameplay.
	// No pointer or reference parameters or return value; updates the observed state's world.
	void OnGameplaySessionResume();

	// Pauses gameplay and announces the pause workflow.
	// No pointer or reference parameters or return value; updates the observed state's world.
	void OnGameplaySessionPaused();

	// Pauses gameplay and announces a victory or defeat.
	// bVictory is passed by value without pointer or reference ownership.
	void OnGameplaySessionEnded(bool bVictory);

private:
	enum class ESessionState : uint8 { Ready, Playing, Paused, Ended };

	// Tracks which lifecycle requests the session can accept.
	// Stores the private enum by value without pointer or reference ownership.
	ESessionState SessionState = ESessionState::Ready;

	// Observes the world's shared bounce state throughout the session.
	// A weak UObject pointer observes the world-owned actor without retaining its lifetime.
	TWeakObjectPtr<AFeatureGameState> GameState;

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
