#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FeatureSaveGame.generated.h"

/** Serialized data for the training sandbox; independent of actors and widgets. */
UCLASS()
class TESTFEATURES_API UFeatureSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	// Preserves the accumulated bounce total between level and game sessions.
	// int32 stores a value snapshot; UPROPERTY(SaveGame) marks the field as persistent without retaining actor pointers.
	UPROPERTY(SaveGame)
	int32 TotalBounces = 0;
};
