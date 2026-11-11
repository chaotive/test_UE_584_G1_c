#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FeatureGameplaySession.generated.h"

class AFeatureGameState;
class UFeatureBallManager;
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
	void BindGameState(AFeatureGameState& State, int32 ContactsToWin, int32 BouncesToLose, int32 ContactsIncrease, int32 BouncesDecrease, int32 MinimumBouncesToLose);

	// Removes this subsystem's observation when its world shuts down.
	// Releases weak UObject subscriptions.
	virtual void Deinitialize() override;

	// Starts the player workflow, initializes progress, and creates the enclosure and ball volley.
	// Configuration values are passed by value; both FBox references borrow world-space bounds.
	void OnGameplaySessionStart(bool bContinue, int32 InitialLevel, int32 BallCount, float BallSize, float BallSpeedMultiplier, int32 ReboundLimit, const FBox& SpawnBounds, const FBox& LevelBounds);

	// Unpauses the active session and announces its return to gameplay.
	// Updates the observed state's world.
	void OnGameplaySessionResume();

	// Pauses gameplay and announces the pause workflow.
	// Updates the observed state's world.
	void OnGameplaySessionPaused();

	// Ends gameplay, records the outcome, and announces victory or defeat to observers.
	void OnGameplaySessionEnded(bool bVictory);

	// Reads the current session's player-contact victory threshold.
	int32 GetPlayerContactsToWin() const;

	// Reads the current session's non-player bounce defeat threshold.
	int32 GetSessionBouncesToLose() const;

private:
	// Grants focused automation checks access to loss-limit behavior.
	// The test classes retain no runtime objects and are compiled only with development automation.
	friend class FFeatureSmokeSpec;

	enum class ESessionState : uint8 { Ready, Playing, Paused, Ended };

	// Tracks which lifecycle requests the session can accept.
	ESessionState SessionState = ESessionState::Ready;

	// Sets the level-one player contact limit, increased by the configured amount for each subsequent level.
	int32 PlayerContactsToWin = 0;

	// Sets the level-one bounce limit, decreased per level down to the configured minimum.
	int32 SessionBouncesToLose = 0;

	// Sets the lowest non-player bounce defeat limit for any level.
	int32 MinimumSessionBouncesToLose = 0;

	// Sets the per-level increase in required player contacts.
	int32 ContactsIncreasePerLevel = 0;

	// Sets the per-level decrease in allowed non-player bounces.
	int32 BouncesDecreasePerLevel = 0;

	// Ends the current session once it reaches its victory or defeat limit.
	// Stats borrows a read-only state-owned session record without transferring ownership.
	void EvaluateSessionLimits(const FFeatureGameplaySessionRecord& Stats);

	// Rechecks deferred statistics once persistence releases its gameplay pause.
	// Status is a borrowed string reference and bBusy is passed by value without ownership.
	void HandlePersistenceStatusChanged(const FString& Status, bool bBusy);

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

	// Coordinates the world-owned ball manager through session lifecycle requests.
	// TWeakObjectPtr observes the world subsystem without retaining its lifetime.
	TWeakObjectPtr<UFeatureBallManager> BallManager;

	// Reports whether persistence currently owns the world's pause state.
	bool IsPersistenceBusy() const;

	// Reports whether gameplay can accept a pause or end request.
	bool CanLeaveGameplay() const;
};
