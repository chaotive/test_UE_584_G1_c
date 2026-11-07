#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FeatureGameState.generated.h"

/** Holds the accumulated training ball bounce total, restored and saved at session boundaries. */
UCLASS()
class TESTFEATURES_API AFeatureGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	DECLARE_EVENT_OneParam(AFeatureGameState, FTotalBouncesChangedEvent, int32);

	// Notifies listeners whenever the shared bounce total changes.
	// This native multicast event passes int32 by value; only the game state can broadcast it.
	FTotalBouncesChangedEvent OnTotalBouncesChanged;

	// Adds one ball bounce to the session total and logs the updated count.
	// No pointer or reference parameters or return value; modifies this world-owned game state.
	void RecordBounce();

	// Reads the session's total bounce count for C++ or Blueprint callers.
	// Returns int32 by value without pointers or references; const keeps this query read-only.
	UFUNCTION(BlueprintPure, Category="Training_Ball")
	int32 GetTotalBounces() const;

	// Requests an in-game save and optionally an orderly quit after completion.
	// bQuitAfter is passed by value; no actor or widget ownership is transferred.
	UFUNCTION(BlueprintCallable, Category="Training_Ball")
	void SaveSession(bool bQuitAfter = false);

	// Starts a new session at zero or requests restoration of the saved total.
	// bContinue is passed by value without pointer or reference ownership.
	void StartSession(bool bContinue);

protected:

	// Subscribes to restored state before the player starts a session.
	// No pointer or reference parameters or return value; operates on this world-owned game state.
	virtual void BeginPlay() override;

	// Saves and logs the final bounce count when the level or game session ends.
	// EndPlayReason is passed by value; no pointer or reference ownership is transferred.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// Allows teardown saving once a new or continued session has an initialized total.
	// bool records session readiness by value without pointer ownership.
	bool bSessionInitialized = false;

	// Initializes a new or restored total and notifies observers.
	// Count is passed by value without pointer or reference ownership.
	void InitializeTotal(int32 Count);

	// Accumulates bounces from all balls, including balls reused from the pool.
	// int32 stores the count by value; UPROPERTY exposes it for inspection and Blueprint reads without allowing edits.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Training_Ball", meta=(AllowPrivateAccess="true"))
	int32 TotalBounces = 0;
};
