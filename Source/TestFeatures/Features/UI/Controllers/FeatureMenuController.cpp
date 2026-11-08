#include "FeatureMenuController.h"
#include "Features/UI/Archetypes/FeatureMainMenu.h"
#include "Features/GameWorkflow/FeatureGameplaySession.h"
#include "Features/GameWorkflow/FeatureProgressPersistence.h"
#include "Features/GameWorkflow/FeatureSaveGameSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UFeatureMenuController::PresentFlow(EFeatureGameFlow Flow)
{
	if (FlowMenu)
	{
		FlowMenu->OnAction.RemoveAll(this);
		FlowMenu->Dismiss();
		FlowMenu = nullptr;
	}
	PresentedFlow = Flow;
	if (Flow != EFeatureGameFlow::Gameplay)
	{
		FlowMenu = UFeatureMainMenu::Present(*GetWorld()->GetGameInstance(), Flow);
		FlowMenu->OnAction.AddUObject(this, &UFeatureMenuController::Execute);
	}
}

void UFeatureMenuController::Execute(EFeatureMenuAction Action)
{
	UFeatureSaveGameSubsystem* Saves = GetWorld()->GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>();
	if (Saves->IsBusy())
	{
		return;
	}
	switch (Action)
	{
	case EFeatureMenuAction::NewGame:
		StartSession(false);
		break;
	case EFeatureMenuAction::Continue:
		StartSession(true);
		break;
	case EFeatureMenuAction::Resume:
		GetWorld()->GetSubsystem<UFeatureGameplaySession>()->OnGameplaySessionResume();
		break;
	case EFeatureMenuAction::DeleteSave:
		Saves->DeleteSave();
		break;
	case EFeatureMenuAction::ReturnToMenu:
		UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
		break;
	case EFeatureMenuAction::Retry:
		UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)), true, TEXT("FeatureRetry=1"));
		break;
	case EFeatureMenuAction::Exit:
		ExitGame();
		break;
	}
}

void UFeatureMenuController::StartSession(bool bContinue)
{
	UFeatureSaveGameSubsystem* Saves = GetWorld()->GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>();
	const bool bCanStart = bContinue ? Saves->HasSave() : Saves->DeleteSave();
	if (bCanStart)
	{
		OnStartRequested.Broadcast(bContinue);
	}
}

void UFeatureMenuController::ExitGame()
{
	if (PresentedFlow == EFeatureGameFlow::StartupMenu)
	{
		UKismetSystemLibrary::QuitGame(this, GetWorld()->GetGameInstance()->GetFirstLocalPlayerController(), EQuitPreference::Quit, false);
	}
	else
	{
		GetWorld()->GetGameState()->FindComponentByClass<UFeatureProgressPersistence>()->SaveSession(true);
	}
}

EFeatureGameFlow UFeatureMenuController::GetFlowState() const
{
	return PresentedFlow;
}

void UFeatureMenuController::Deinitialize()
{
	if (FlowMenu)
	{
		FlowMenu->OnAction.RemoveAll(this);
		FlowMenu->RemoveFromParent();
		FlowMenu = nullptr;
	}
	Super::Deinitialize();
}
