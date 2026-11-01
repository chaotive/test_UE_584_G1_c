// Copyright Epic Games, Inc. All Rights Reserved.

#include "training_UE_584_G2_cGameMode.h"
#include "training_UE_584_G2_c.h"
#include "Features/TrainingGameState.h"
#include "Features/TrainingGameStateController.h"

Atraining_UE_584_G2_cGameMode::Atraining_UE_584_G2_cGameMode()
{
	// Selects TrainingGameState as the class Unreal will instantiate for this game mode.
	// StaticClass() returns its UClass metadata pointer, not an actor instance; GameStateClass stores this as a TSubclassOf<AGameStateBase>.
	// AGameModeBase::PreInitializeComponents() spawns this actor, registers it with the world, then calls InitGameState(), before BeginPlay().
	GameStateClass = ATrainingGameState::StaticClass();
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("GameMode [%s]: Constructor"), *GetName());
}

void Atraining_UE_584_G2_cGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("GameMode [%s]: InitGame (Map: %s)"), *GetName(), *MapName);
	Super::InitGame(MapName, Options, ErrorMessage);
}

void Atraining_UE_584_G2_cGameMode::InitGameState()
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("GameMode [%s]: InitGameState"), *GetName());
	Super::InitGameState();

	// The base game mode has already spawned the game state before calling this hook.
	// NewObject creates the controller with this game mode as its Outer; our UPROPERTY keeps it alive.
	TrainingGameStateController = NewObject<UTrainingGameStateController>(this);
	// Give the controller the existing shared state; it does not create another game state actor.
	TrainingGameStateController->Initialize(*GetGameState<ATrainingGameState>());
	// Subscribe before BeginPlay spawns balls; each new pool entry will be passed to ObserveBall.
	// AddUObject binds the method to this controller using a weak UObject reference.
	TrainingBallSpawner.OnBallCreated.AddUObject(TrainingGameStateController.Get(), &UTrainingGameStateController::ObserveBall);
}

void Atraining_UE_584_G2_cGameMode::PostInitializeComponents()
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("GameMode [%s]: PostInitializeComponents"), *GetName());
	Super::PostInitializeComponents();
}

void Atraining_UE_584_G2_cGameMode::StartPlay()
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("GameMode [%s]: StartPlay"), *GetName());
	Super::StartPlay();
}

void Atraining_UE_584_G2_cGameMode::BeginPlay()
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("GameMode [%s]: BeginPlay"), *GetName());
	Super::BeginPlay();

	TrainingBallSpawner.SpawnMultiple(*GetWorld(), TrainingBallCount, TrainingBallSpawnMin, TrainingBallSpawnMax);
}

void Atraining_UE_584_G2_cGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("GameMode [%s]: EndPlay (Reason: %s)"),
		*GetName(), *UEnum::GetValueAsString(EndPlayReason));
	Super::EndPlay(EndPlayReason);
}
