#pragma once

#include "CoreMinimal.h"

class APlayerController;
class UFeatureDebugHUD;

/** Creates and displays a debug overlay independently of gameplay state or ball spawning. */
class TESTFEATURES_API FFeatureDebugHUDSpawner
{
public:

	// Creates the native debug widget and adds it above the owning player's game view.
	// PlayerController is a mutable reference; the raw return pointer refers to a widget retained by the caller's UPROPERTY.
	static UFeatureDebugHUD* Spawn(APlayerController& PlayerController);
};
