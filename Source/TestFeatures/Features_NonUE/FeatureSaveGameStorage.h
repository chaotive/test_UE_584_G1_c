#pragma once

#include "CoreMinimal.h"
#include "Features/GameWorkflow/FeatureSaveGame.h"
#include "Misc/Optional.h"

/** Stores value snapshots without depending on the live game state or HUD. */
class TESTFEATURES_API FFeatureSaveGameStorage
{
public:

	// Reports whether the local slot has data for a Continue attempt.
	// Returns bool by value without pointer or reference ownership.
	static bool HasSave();

	// Deletes the local slot, treating an already empty slot as success.
	// Returns bool by value without pointer or reference ownership.
	static bool DeleteSave();

	// Loads a snapshot without blocking rendering while disk data is read.
	// TFunction owns the callback; TOptional owns a value snapshot or marks unavailable data, and the diagnostic string is borrowed.
	static void LoadAsync(TFunction<void(TOptional<FFeatureGameProgress>, const FString&)> Completion);

	// Saves a complete progress snapshot while the game thread continues drawing UI.
	// Progress is borrowed read-only and copied into the save object; Completion owns its callback.
	static void SaveAsync(const FFeatureGameProgress& Progress, TFunction<void(bool)> Completion);

	// Reads the saved bounce total, or returns zero when no readable save exists.
	// Result is a mutable string reference for diagnostics; returns int32 by value without transferring object ownership.
	static int32 LoadTotalBounces(FString& Result);

	// Identifies the local slot used by both persistence and diagnostics.
	// Returns a const FString reference to process-lifetime value storage without transferring ownership.
	static const FString& GetSlotName();

	// Reports the desktop save file corresponding to the local slot.
	// Returns an absolute FString path by value without pointer ownership.
	static FString GetSaveFilePath();

	// Writes progress to the sandbox's save slot before shutdown.
	// Progress is borrowed read-only and serialized by value; bool reports success without object ownership.
	static bool SaveProgress(const FFeatureGameProgress& Progress);
};
