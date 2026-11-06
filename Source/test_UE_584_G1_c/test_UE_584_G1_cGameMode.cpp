// Copyright Epic Games, Inc. All Rights Reserved.

#include "test_UE_584_G1_cGameMode.h"
#include "test_UE_584_G1_c.h"
#include "Features/TrainingGameState.h"
#include "Features/TrainingGameStateController.h"

Atest_UE_584_G1_cGameMode::Atest_UE_584_G1_cGameMode()
{
	// Selects TrainingGameState as the class Unreal will instantiate for this game mode.
	// StaticClass() returns its UClass metadata pointer, not an actor instance; GameStateClass stores this as a TSubclassOf<AGameStateBase>.
	// AGameModeBase::PreInitializeComponents() spawns this actor, registers it with the world, then calls InitGameState(), before BeginPlay().
	GameStateClass = ATrainingGameState::StaticClass();
	UE_LOG(Logtest_UE_584_G1_c, Log, TEXT("GameMode [%s]: Constructor"), *GetName());
}

void Atest_UE_584_G1_cGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	UE_LOG(Logtest_UE_584_G1_c, Log, TEXT("GameMode [%s]: InitGame (Map: %s)"), *GetName(), *MapName);
	Super::InitGame(MapName, Options, ErrorMessage);
}

void Atest_UE_584_G1_cGameMode::InitGameState()
{
	UE_LOG(Logtest_UE_584_G1_c, Log, TEXT("GameMode [%s]: InitGameState"), *GetName());
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

void Atest_UE_584_G1_cGameMode::PostInitializeComponents()
{
	UE_LOG(Logtest_UE_584_G1_c, Log, TEXT("GameMode [%s]: PostInitializeComponents"), *GetName());
	Super::PostInitializeComponents();
}

void Atest_UE_584_G1_cGameMode::StartPlay()
{
	UE_LOG(Logtest_UE_584_G1_c, Log, TEXT("GameMode [%s]: StartPlay"), *GetName());
	Super::StartPlay();
}

void Atest_UE_584_G1_cGameMode::BeginPlay()
{
	UE_LOG(Logtest_UE_584_G1_c, Log, TEXT("GameMode [%s]: BeginPlay"), *GetName());
	Super::BeginPlay();

	TrainingBallSpawner.SpawnMultiple(*GetWorld(), TrainingBallCount, TrainingBallSpawnMin, TrainingBallSpawnMax);
}

void Atest_UE_584_G1_cGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(Logtest_UE_584_G1_c, Log, TEXT("GameMode [%s]: EndPlay (Reason: %s)"),
		*GetName(), *UEnum::GetValueAsString(EndPlayReason));
	Super::EndPlay(EndPlayReason);
}
