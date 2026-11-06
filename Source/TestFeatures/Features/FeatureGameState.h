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

protected:

	// Starts paused in-game loading and listens for restored state.
	// No pointer or reference parameters or return value; operates on this world-owned game state.
	virtual void BeginPlay() override;

	// Saves and logs the final bounce count when the level or game session ends.
	// EndPlayReason is passed by value; no pointer or reference ownership is transferred.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// Prevents forced teardown during loading from overwriting saved progress with an uninitialized total.
	// bool records per-level restoration state by value without pointer ownership.
	bool bHasRestoredTotal = false;

	// Applies a restored total before the save subsystem releases gameplay.
	// Count is passed by value without pointer or reference ownership.
	void ApplyLoadedTotal(int32 Count);

	// Accumulates bounces from all balls, including balls reused from the pool.
	// int32 stores the count by value; UPROPERTY exposes it for inspection and Blueprint reads without allowing edits.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Training_Ball", meta=(AllowPrivateAccess="true"))
	int32 TotalBounces = 0;
};
