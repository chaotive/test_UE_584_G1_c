#pragma once

#include "CoreMinimal.h"

/** Stores value snapshots without depending on the live game state or HUD. */
class TESTFEATURES_API FFeatureSaveGameStorage
{
public:

	// Loads a snapshot without blocking rendering while disk data is read.
	// Completion is an owned TFunction callback receiving the count by value and a read-only result reference.
	static void LoadAsync(TFunction<void(int32, const FString&)> Completion);

	// Saves an integer snapshot while the game thread continues drawing UI.
	// Count is by value and Completion is an owned callback receiving a bool result; no actors are captured by storage.
	static void SaveAsync(int32 Count, TFunction<void(bool)> Completion);

	// Reads the saved bounce total, or returns zero when no readable save exists.
	// Result is a mutable string reference for diagnostics; returns int32 by value without transferring object ownership.
	static int32 LoadTotalBounces(FString& Result);

	// Identifies the local slot used by both persistence and diagnostics.
	// Returns a const FString reference to process-lifetime value storage without transferring ownership.
	static const FString& GetSlotName();

	// Reports the desktop save file corresponding to the local slot.
	// Returns an absolute FString path by value without pointer ownership.
	static FString GetSaveFilePath();

	// Writes a bounce-total snapshot to the sandbox's save slot before shutdown.
	// TotalBounces and the bool success result are passed by value; no live actor references are serialized.
	static bool SaveTotalBounces(int32 TotalBounces);
};
