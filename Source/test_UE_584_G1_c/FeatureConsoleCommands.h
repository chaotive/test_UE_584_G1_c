#pragma once

#include "CoreTypes.h"

#if !UE_BUILD_SHIPPING

class FAutoConsoleCommandWithWorld;
class UWorld;

class FFeatureConsoleCommands
{
	// Registers the console shortcut for starting gameplay from the startup menu.
	// Stores the registration by value with static lifetime and no pointer ownership.
	static FAutoConsoleCommandWithWorld NewGameCommand;

	// Starts a fresh session when the command targets a world at the startup menu.
	// World is a nullable, borrowed engine-owned pointer. This function returns void.
	static void StartNewGame(UWorld* World);
};

#endif
