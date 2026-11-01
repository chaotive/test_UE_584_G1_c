// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Features/TrainingBallSpawner.h"
#include "training_UE_584_G2_cGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class Atraining_UE_584_G2_cGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	// No pointer or reference parameters; Unreal creates and owns this game mode instance.
	Atraining_UE_584_G2_cGameMode();

	// MapName and Options are read-only references; ErrorMessage is a mutable reference for reporting initialization errors.
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	// No pointer or reference parameters; initializes the game state owned by the world.
	virtual void InitGameState() override;
	// No pointer or reference parameters; starts play for this game mode's world.
	virtual void StartPlay() override;

protected:

	// No pointer or reference parameters; completes this actor's component initialization.
	virtual void PostInitializeComponents() override;
	// No pointer or reference parameters; spawns world-owned training balls when play begins.
	virtual void BeginPlay() override;
	// EndPlayReason is passed by value; no pointer or reference ownership is transferred.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Number of training balls to create when play begins.
	// int32 stores the ball count by value; UPROPERTY(EditAnywhere) exposes it for editor configuration without pointer ownership.
	UPROPERTY(EditAnywhere, Category="Training_Ball")
	int32 TrainingBallCount = 5;

	// Minimum corner of the spawn box in world centimeters; each axis must be <= the maximum corner.
	// FVector stores three coordinates by value; UPROPERTY exposes them for editing without a pointer or separate allocation.
	UPROPERTY(EditAnywhere, Category="Training_Ball")
	FVector TrainingBallSpawnMin = FVector(-500.0f, -300.0f, 400.0f);

	// Maximum corner of the spawn box in world centimeters.
	// FVector stores three coordinates by value; UPROPERTY exposes them for editing without a pointer or separate allocation.
	UPROPERTY(EditAnywhere, Category="Training_Ball")
	FVector TrainingBallSpawnMax = FVector(500.0f, 700.0f, 700.0f);

private:

	// Creates training balls and tracks them throughout this game mode's lifetime.
	// FTrainingBallSpawner is stored by value so its list belongs to this game mode instance without a pointer or separate allocation.
	FTrainingBallSpawner TrainingBallSpawner;
};
