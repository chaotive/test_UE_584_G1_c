// Copyright Epic Games, Inc. All Rights Reserved.

#include "test_UE_584_G1_cGameMode.h"
#include "test_UE_584_G1_c.h"
#include "test_UE_584_G1_cCharacter.h"
#include "Features/Gameplay/FeatureGameState.h"
#include "Features/GameWorkflow/FeatureGameplaySession.h"
#include "Features/UI/Archetypes/FeatureMainMenu.h"
#include "Features/GameWorkflow/FeatureSaveGameSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

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
	bRetrySession = UGameplayStatics::HasOption(Options, TEXT("FeatureRetry"));
}

void Atest_UE_584_G1_cGameMode::InitGameState()
{
	UE_LOG(Logtest_UE_584_G1_c, Log, TEXT("GameMode [%s]: InitGameState"), *GetName());
	Super::InitGameState();

	// Blueprint previews have no gameplay session or game instance to bind.
	if (!GetWorld()->IsGameWorld())
	{
		return;
	}

	GameplaySession = GetWorld()->GetSubsystem<UFeatureGameplaySession>();
	GameplaySession->BindGameState(*GetGameState<AFeatureGameState>(), FeaturePlayerContactsToWin, FeatureSessionBouncesToLose,
		FeatureContactsIncreasePerLevel, FeatureBouncesDecreasePerLevel);
	GameplaySession->OnStarted.AddUObject(this, &Atest_UE_584_G1_cGameMode::HandleGameplaySessionStarted);
	GameplaySession->OnResumed.AddUObject(this, &Atest_UE_584_G1_cGameMode::HandleGameplaySessionResumed);
	GameplaySession->OnPaused.AddUObject(this, &Atest_UE_584_G1_cGameMode::HandleGameplaySessionPaused);
	GameplaySession->OnEnded.AddUObject(this, &Atest_UE_584_G1_cGameMode::HandleGameplaySessionEnded);
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
	if (bRetrySession)
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &Atest_UE_584_G1_cGameMode::StartRetrySession);
	}
}

void Atest_UE_584_G1_cGameMode::StartRetrySession()
{
	HandleMenuAction(EFeatureMenuAction::Continue);
}

void Atest_UE_584_G1_cGameMode::BeginPlay()
{
	UE_LOG(Logtest_UE_584_G1_c, Log, TEXT("GameMode [%s]: BeginPlay"), *GetName());
	Super::BeginPlay();
}

void Atest_UE_584_G1_cGameMode::HandleGameplaySessionStarted()
{
	SetFlow(EFeatureGameFlow::Gameplay);
	APlayerController* Player = GetGameInstance()->GetFirstLocalPlayerController();
	RestartPlayer(Player);
	CastChecked<Atest_UE_584_G1_cCharacter>(Player->GetPawn())->ConfigureFeatureSpeed(
		FeaturePlayerBaseSpeedMultiplier, FeaturePlayerContactSpeedMultiplier, FeaturePlayerContactBoostDuration, bFeaturePlayerContactBoostStacks);
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

void Atest_UE_584_G1_cGameMode::PauseSession()
{
	GameplaySession->OnGameplaySessionPaused();
}

void Atest_UE_584_G1_cGameMode::FinishSession(bool bVictory)
{
	GameplaySession->OnGameplaySessionEnded(bVictory);
}

void Atest_UE_584_G1_cGameMode::HandleGameplaySessionResumed()
{
	SetFlow(EFeatureGameFlow::Gameplay);
}

void Atest_UE_584_G1_cGameMode::HandleGameplaySessionPaused()
{
	SetFlow(EFeatureGameFlow::PauseMenu);
}

void Atest_UE_584_G1_cGameMode::HandleGameplaySessionEnded(bool bVictory)
{
	SetFlow(bVictory ? EFeatureGameFlow::Victory : EFeatureGameFlow::Lose);
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
		FlowMenu = UFeatureMainMenu::Present(*GetGameInstance(), FlowState);
		FlowMenu->OnAction.AddUObject(this, &Atest_UE_584_G1_cGameMode::HandleMenuAction);
	}
}

void Atest_UE_584_G1_cGameMode::HandleMenuAction(EFeatureMenuAction Action)
{
	UFeatureSaveGameSubsystem* Saves = GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>();
	if (Saves->IsBusy())
	{
		return;
	}

	switch (Action)
	{
	case EFeatureMenuAction::NewGame:
		if (!Saves->DeleteSave())
		{
			break;
		}
		GameplaySession->OnGameplaySessionStart(false, FeatureInitialLevel, FeatureBallCount, FeatureBallSize, FeatureBallSpeedMultiplier, FeatureBallReboundsBeforeRetirement,
			FBox(FeatureBallSpawnMin, FeatureBallSpawnMax),
			FBox::BuildAABB(FeatureLevelBoundsCenter, FeatureLevelBoundsHalfExtent));
		break;
	case EFeatureMenuAction::Continue:
		if (!Saves->HasSave())
		{
			break;
		}
		GameplaySession->OnGameplaySessionStart(true, FeatureInitialLevel, FeatureBallCount, FeatureBallSize, FeatureBallSpeedMultiplier, FeatureBallReboundsBeforeRetirement,
			FBox(FeatureBallSpawnMin, FeatureBallSpawnMax),
			FBox::BuildAABB(FeatureLevelBoundsCenter, FeatureLevelBoundsHalfExtent));
		break;
	case EFeatureMenuAction::Resume:
		GameplaySession->OnGameplaySessionResume();
		break;
	case EFeatureMenuAction::ReturnToMenu:
		UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
		break;
	case EFeatureMenuAction::Retry:
		UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)), true, TEXT("FeatureRetry=1"));
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
	if (GameplaySession)
	{
		GameplaySession->OnStarted.RemoveAll(this);
		GameplaySession->OnResumed.RemoveAll(this);
		GameplaySession->OnPaused.RemoveAll(this);
		GameplaySession->OnEnded.RemoveAll(this);
	}
	if (FlowMenu)
	{
		FlowMenu->OnAction.RemoveAll(this);
		FlowMenu->RemoveFromParent();
	}
	UE_LOG(Logtest_UE_584_G1_c, Log, TEXT("GameMode [%s]: EndPlay (Reason: %s)"),
		*GetName(), *UEnum::GetValueAsString(EndPlayReason));
	Super::EndPlay(EndPlayReason);
}
