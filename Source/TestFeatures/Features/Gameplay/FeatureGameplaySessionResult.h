#pragma once

#include "CoreMinimal.h"
#include "FeatureGameplaySessionResult.generated.h"

// Identifies the outcome recorded with a gameplay session's statistics.
// BlueprintType exposes the reflected result as a Blueprint value.
UENUM(BlueprintType)
enum class EFeatureGameplaySessionResult : uint8
{
	InProgress,
	Victory,
	Defeat
};
