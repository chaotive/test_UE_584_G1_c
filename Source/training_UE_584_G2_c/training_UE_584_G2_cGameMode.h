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

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void StartPlay() override;

protected:

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Center of the ball when the game starts (in centimeters). */
	UPROPERTY(EditAnywhere, Category="Training_Ball")
	FVector TrainingBallSpawnLocation = FVector(0.0f, 200.0f, 400.0f);
};
