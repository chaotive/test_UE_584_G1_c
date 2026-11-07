#include "FeatureConsoleCommands.h"

#if !UE_BUILD_SHIPPING

#include "Engine/World.h"
#include "Features/FeatureMainMenu.h"
#include "HAL/IConsoleManager.h"
#include "test_UE_584_G1_cGameMode.h"

FAutoConsoleCommandWithWorld FFeatureConsoleCommands::NewGameCommand(
	TEXT("Feature.NewGame"),
	TEXT("Start a fresh session immediately from the startup menu."),
	FConsoleCommandWithWorldDelegate::CreateStatic(&FFeatureConsoleCommands::StartNewGame));

void FFeatureConsoleCommands::StartNewGame(UWorld* World)
{
	auto* GameMode = World ? World->GetAuthGameMode<Atest_UE_584_G1_cGameMode>() : nullptr;
	if (GameMode && GameMode->GetFlowState() == EFeatureGameFlow::StartupMenu)
	{
		GameMode->HandleMenuAction(EFeatureMenuAction::NewGame);
	}
}

#endif
