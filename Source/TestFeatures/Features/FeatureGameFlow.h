#pragma once

#include "CoreMinimal.h"
#include "FeatureGameFlow.generated.h"

/** Identifies the current phase of the game session. */
UENUM(BlueprintType)
enum class EFeatureGameFlow : uint8
{
	StartupMenu,
	Gameplay,
	PauseMenu,
	Victory,
	Lose
};
