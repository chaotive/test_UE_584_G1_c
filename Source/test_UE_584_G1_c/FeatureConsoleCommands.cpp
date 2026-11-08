#include "FeatureConsoleCommands.h"

#if !UE_BUILD_SHIPPING

#include "Engine/World.h"
#include "Features/UI/Controllers/FeatureMenuController.h"
#include "Features/UI/Controllers/FeatureMenuActions.h"
#include "Features/GameWorkflow/FeatureGameFlow.h"
#include "HAL/IConsoleManager.h"

FAutoConsoleCommandWithWorld FFeatureConsoleCommands::NewGameCommand(
	TEXT("Feature.NewGame"),
	TEXT("Start a fresh session immediately from the startup menu."),
	FConsoleCommandWithWorldDelegate::CreateStatic(&FFeatureConsoleCommands::StartNewGame));

void FFeatureConsoleCommands::StartNewGame(UWorld* World)
{
	UFeatureMenuController* Menu = World && World->GetAuthGameMode() ? World->GetSubsystem<UFeatureMenuController>() : nullptr;
	if (Menu && Menu->GetFlowState() == EFeatureGameFlow::StartupMenu)
	{
		Menu->Execute(EFeatureMenuAction::NewGame);
	}
}

#endif
