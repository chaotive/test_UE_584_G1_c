#pragma once

#include "CoreMinimal.h"

/** Stores value snapshots without depending on the live game state or HUD. */
class FTrainingSaveGameStorage
{
public:

	// Reads the saved bounce total, or returns zero when no readable save exists.
	// Returns int32 by value without pointers or references; loaded Unreal objects stay local to the operation.
	static int32 LoadTotalBounces();

	// Writes a bounce-total snapshot to the sandbox's save slot before shutdown.
	// TotalBounces and the bool success result are passed by value; no live actor references are serialized.
	static bool SaveTotalBounces(int32 TotalBounces);
};
