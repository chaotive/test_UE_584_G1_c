#pragma once

#include "CoreMinimal.h"
#include "Features/GameWorkflow/FeatureSaveGame.h"
#include "GameFramework/GameStateBase.h"
#include "FeatureGameState.generated.h"

/** Holds saved-game progress and the statistics for every started gameplay session. */
UCLASS()
class TESTFEATURES_API AFeatureGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	DECLARE_EVENT_OneParam(AFeatureGameState, FTotalBouncesChangedEvent, int32);
	DECLARE_EVENT_OneParam(AFeatureGameState, FSessionStatsChangedEvent, const FFeatureGameplaySessionRecord&);
	// Announces a change in saved level for the HUD and startup menu.
	// The event wrapper passes int32 by value and stores weak UObject bindings.
	DECLARE_EVENT_OneParam(AFeatureGameState, FCurrentLevelChangedEvent, int32);

	// Notifies listeners whenever the shared bounce total changes.
	// This native multicast event passes int32 by value; only the game state can broadcast it.
	FTotalBouncesChangedEvent OnTotalBouncesChanged;

	// Announces updated statistics for the current gameplay session.
	// The native event borrows a read-only reference to the state-owned record.
	FSessionStatsChangedEvent OnSessionStatsChanged;

	// Notifies UI and gameplay observers when the saved level changes.
	// This native multicast event passes int32 by value and stores weak UObject bindings.
	FCurrentLevelChangedEvent OnCurrentLevelChanged;

	// Adds a bounce to the saved-game total and active session record, then notifies observers.
	// Modifies this world-owned game state.
	void RecordBounce();

	// Records one player-ball contact in the active session.
	// Increments a value counter.
	void RecordPlayerBallCollision();

	// Stores the current session's outcome and stops accepting further gameplay statistics.
	void RecordSessionResult(bool bVictory);

	// Exposes the saved game's chronological session history for inspection.
	// Returns a borrowed const TArray reference to state-owned value records.
	const TArray<FFeatureGameplaySessionRecord>& GetGameplaySessions() const;

	// Reads the saved level to start next or retry after defeat.
	int32 GetCurrentLevel() const;

	// Reads the saved game's aggregate bounce count for C++ or Blueprint callers.
	// Returns int32 by value without pointers or references; const keeps this query read-only.
	UFUNCTION(BlueprintPure, Category="Training_Ball")
	int32 GetTotalBounces() const;

	// Appends the current session to a supplied progress snapshot and notifies observers.
	// Snapshot is a borrowed mutable reference so a queued save-after-load receives the initialized values.
	void InitializeSession(FFeatureGameProgress& Snapshot);

	// Exposes current progress for inspection and snapshot copying by workflow consumers.
	// Returns a borrowed const reference to state-owned values without transferring ownership.
	const FFeatureGameProgress& GetProgress() const;

	// Reports whether progress includes a session initialized for this world.
	bool IsSessionInitialized() const;

protected:

	// Logs state startup while workflow bindings initialize independently.
	// Operates on this world-owned game state.
	virtual void BeginPlay() override;

	// Logs the final bounce count and dispatches native actor teardown.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// Distinguishes initialized session records from the state's default values.
	bool bSessionInitialized = false;

	// Retains the aggregate bounce total and every started session's statistics.
	// VisibleAnywhere allows editor inspection. BlueprintReadOnly with AllowPrivateAccess gives Blueprint readers access while C++ owns mutations.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Training_Ball", meta=(AllowPrivateAccess="true"))
	FFeatureGameProgress Progress;
};
