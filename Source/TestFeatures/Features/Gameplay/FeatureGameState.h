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
	// No pointer or reference parameters or return value; modifies this world-owned game state.
	void RecordBounce();

	// Records one player-ball contact in the active session.
	// No pointer or reference parameters or return value; increments a value counter.
	void RecordPlayerBallCollision();

	// Stores the current session's outcome and stops accepting further gameplay statistics.
	// bVictory is passed by value without pointer or reference ownership.
	void RecordSessionResult(bool bVictory);

	// Exposes the saved game's chronological session history for inspection.
	// Returns a borrowed const TArray reference to state-owned value records.
	const TArray<FFeatureGameplaySessionRecord>& GetGameplaySessions() const;

	// Reads the saved level to start next or retry after defeat.
	// Returns int32 by value without pointers or references.
	int32 GetCurrentLevel() const;

	// Reads the saved game's aggregate bounce count for C++ or Blueprint callers.
	// Returns int32 by value without pointers or references; const keeps this query read-only.
	UFUNCTION(BlueprintPure, Category="Training_Ball")
	int32 GetTotalBounces() const;

	// Requests an in-game save and optionally an orderly quit after completion.
	// bQuitAfter is passed by value; no actor or widget ownership is transferred.
	UFUNCTION(BlueprintCallable, Category="Training_Ball")
	void SaveSession(bool bQuitAfter = false);

	// Starts a fresh saved game or loads progress before appending the current session.
	// bContinue and InitialLevel are passed by value without pointer or reference ownership.
	void StartSession(bool bContinue, int32 InitialLevel);

protected:

	// Subscribes to restored state before the player starts a session.
	// No pointer or reference parameters or return value; operates on this world-owned game state.
	virtual void BeginPlay() override;

	// Saves and logs the final bounce count when the level or game session ends.
	// EndPlayReason is passed by value; no pointer or reference ownership is transferred.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// Allows teardown saving once a new or continued session has initialized progress.
	// bool records session readiness by value without pointer ownership.
	bool bSessionInitialized = false;

	// Appends the current session to a fresh or restored snapshot and notifies observers.
	// Snapshot is a mutable borrowed reference so queued save-after-load uses initialized progress.
	void InitializeProgress(FFeatureGameProgress& Snapshot);

	// Retains the aggregate bounce total and every started session's statistics.
	// The reflected struct owns its values and array; UPROPERTY exposes read-only Blueprint inspection.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Training_Ball", meta=(AllowPrivateAccess="true"))
	FFeatureGameProgress Progress;
};
