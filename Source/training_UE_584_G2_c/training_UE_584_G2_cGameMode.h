// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "training_UE_584_G2_cGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class Atraining_UE_584_G2_cGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	Atraining_UE_584_G2_cGameMode();

protected:

	virtual void BeginPlay() override;

	/** Center of the ball when the game starts (in centimeters). */
	UPROPERTY(EditAnywhere, Category="Ball")
	FVector BallSpawnLocation = FVector(0.0f, 0.0f, 400.0f);
};



