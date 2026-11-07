// Copyright Epic Games, Inc. All Rights Reserved.

#include "test_UE_584_G1_cGameMode.h"
#include "test_UE_584_G1_c.h"
#include "Features/FeatureGameState.h"
#include "Features/FeatureGameStateController.h"
#include "Features/FeatureMainMenu.h"
#include "Features/FeatureSaveGameSubsystem.h"
#include "Features_NonUE/FeatureSaveGameStorage.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

Atest_UE_584_G1_cGameMode::Atest_UE_584_G1_cGameMode()
{
	// Selects FeatureGameState as the class Unreal will instantiate for this game mode.
	// StaticClass() returns its UClass metadata pointer, not an actor instance; GameStateClass stores this as a TSubclassOf<AGameStateBase>.
	// AGameModeBase::PreInitializeComponents() spawns this actor, registers it with the world, then calls InitGameState(), before BeginPlay().
	GameStateClass = AFeatureGameState::StaticClass();
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
	FeatureGameStateController = NewObject<UFeatureGameStateController>(this);
	// Give the controller the existing shared state; it does not create another game state actor.
	FeatureGameStateController->Initialize(*GetGameState<AFeatureGameState>());
	// Subscribe before a session spawns balls; each new pool entry will be passed to ObserveBall.
	// AddUObject binds the method to this controller using a weak UObject reference.
	FeatureBallSpawner.OnBallCreated.AddUObject(FeatureGameStateController.Get(), &UFeatureGameStateController::ObserveBall);
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
	SetFlow(EFeatureGameFlow::StartupMenu);
}

void Atest_UE_584_G1_cGameMode::BeginPlay()
{
	UE_LOG(Logtest_UE_584_G1_c, Log, TEXT("GameMode [%s]: BeginPlay"), *GetName());
	Super::BeginPlay();
}

void Atest_UE_584_G1_cGameMode::StartSession(bool bContinue)
{
	SetFlow(EFeatureGameFlow::Gameplay);
	RestartPlayer(GetGameInstance()->GetFirstLocalPlayerController());
	GetGameState<AFeatureGameState>()->StartSession(bContinue);
	FeatureBallSpawner.SpawnMultiple(*GetWorld(), FeatureBallCount, FeatureBallSpawnMin, FeatureBallSpawnMax);
}

void Atest_UE_584_G1_cGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	if (FlowState == EFeatureGameFlow::Gameplay)
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

EFeatureGameFlow Atest_UE_584_G1_cGameMode::GetFlowState() const
{
	return FlowState;
}

bool Atest_UE_584_G1_cGameMode::CanLeaveGameplay() const
{
	return FlowState == EFeatureGameFlow::Gameplay
		&& !GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>()->IsBusy();
}

void Atest_UE_584_G1_cGameMode::PauseSession()
{
	if (CanLeaveGameplay() && UGameplayStatics::SetGamePaused(this, true))
	{
		SetFlow(EFeatureGameFlow::PauseMenu);
	}
}

void Atest_UE_584_G1_cGameMode::FinishSession(bool bVictory)
{
	if (CanLeaveGameplay() && UGameplayStatics::SetGamePaused(this, true))
	{
		SetFlow(bVictory ? EFeatureGameFlow::Victory : EFeatureGameFlow::Lose);
	}
}

void Atest_UE_584_G1_cGameMode::SetFlow(EFeatureGameFlow NewFlow)
{
	if (FlowMenu)
	{
		FlowMenu->OnAction.RemoveAll(this);
		FlowMenu->Dismiss();
		FlowMenu = nullptr;
	}

	FlowState = NewFlow;
	UE_LOG(Logtest_UE_584_G1_c, Log, TEXT("Game flow: %s"), *UEnum::GetValueAsString(FlowState));
	if (FlowState != EFeatureGameFlow::Gameplay)
	{
		const bool bCanContinue = FlowState == EFeatureGameFlow::StartupMenu && FFeatureSaveGameStorage::HasSave();
		FlowMenu = UFeatureMainMenu::Present(*GetGameInstance(), FlowState, bCanContinue);
		FlowMenu->OnAction.AddUObject(this, &Atest_UE_584_G1_cGameMode::HandleMenuAction);
	}
}

void Atest_UE_584_G1_cGameMode::HandleMenuAction(EFeatureMenuAction Action)
{
	if (GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>()->IsBusy())
	{
		return;
	}

	switch (Action)
	{
	case EFeatureMenuAction::NewGame:
	case EFeatureMenuAction::Continue:
		StartSession(Action == EFeatureMenuAction::Continue);
		break;
	case EFeatureMenuAction::Resume:
		UGameplayStatics::SetGamePaused(this, false);
		SetFlow(EFeatureGameFlow::Gameplay);
		break;
	case EFeatureMenuAction::ReturnToMenu:
		UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
		break;
	case EFeatureMenuAction::Exit:
		if (FlowState == EFeatureGameFlow::StartupMenu)
		{
			UKismetSystemLibrary::QuitGame(this, GetGameInstance()->GetFirstLocalPlayerController(), EQuitPreference::Quit, false);
		}
		else
		{
			GetGameState<AFeatureGameState>()->SaveSession(true);
		}
		break;
	}
}

void Atest_UE_584_G1_cGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (FlowMenu)
	{
		FlowMenu->OnAction.RemoveAll(this);
		FlowMenu->RemoveFromParent();
	}
	UE_LOG(Logtest_UE_584_G1_c, Log, TEXT("GameMode [%s]: EndPlay (Reason: %s)"),
		*GetName(), *UEnum::GetValueAsString(EndPlayReason));
	Super::EndPlay(EndPlayReason);
}
