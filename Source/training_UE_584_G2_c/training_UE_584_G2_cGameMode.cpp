// Copyright Epic Games, Inc. All Rights Reserved.

#include "training_UE_584_G2_cGameMode.h"
#include "Features/TrainingBallSpawner.h"

Atraining_UE_584_G2_cGameMode::Atraining_UE_584_G2_cGameMode()
{
	// stub
}

void Atraining_UE_584_G2_cGameMode::BeginPlay()
{
	Super::BeginPlay();

	FTrainingBallSpawner::Spawn(*GetWorld(), BallSpawnLocation);
}
