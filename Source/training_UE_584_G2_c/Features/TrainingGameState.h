#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TrainingGameState.generated.h"

/** Holds the total number of training ball bounces in the current game session. */
UCLASS()
class ATrainingGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	DECLARE_EVENT_OneParam(ATrainingGameState, FTotalBouncesChangedEvent, int32);

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

protected:

	// Logs the initial bounce count when the game session begins.
	// No pointer or reference parameters or return value; operates on this world-owned game state.
	virtual void BeginPlay() override;

	// Logs the final bounce count when the game session ends.
	// EndPlayReason is passed by value; no pointer or reference ownership is transferred.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// Accumulates bounces from all balls, including balls reused from the pool.
	// int32 stores the count by value; UPROPERTY exposes it for inspection and Blueprint reads without allowing edits.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Training_Ball", meta=(AllowPrivateAccess="true"))
	int32 TotalBounces = 0;
};
