#include "FeatureMenuViews.h"

#define LOCTEXT_NAMESPACE "FeatureMenuViews"

FFeatureMenuView FFeatureMenuViews::ForFlow(EFeatureGameFlow Flow)
{
	FFeatureMenuView View;
	switch (Flow)
	{
	case EFeatureGameFlow::PauseMenu:
		View = PauseMenu();
		break;
	case EFeatureGameFlow::Victory:
		View = VictoryMenu();
		break;
	case EFeatureGameFlow::Lose:
		View = DefeatMenu();
		break;
	default:
		View = InitialMenu();
		break;
	}
	View.Items.Add({EFeatureMenuAction::Exit, LOCTEXT("Exit", "Exit")});
	return View;
}

FFeatureMenuView FFeatureMenuViews::InitialMenu()
{
	return {FText::GetEmpty(), false, true, true, {
		{EFeatureMenuAction::NewGame, LOCTEXT("NewGame", "New Game")},
		{EFeatureMenuAction::Continue, LOCTEXT("ContinueLevelOne", "Continue on level 1")},
		{EFeatureMenuAction::DeleteSave, LOCTEXT("DeleteSave", "Delete Save")}
	}};
}

FFeatureMenuView FFeatureMenuViews::PauseMenu()
{
	return {LOCTEXT("Paused", "Paused"), true, false, true, {
		{EFeatureMenuAction::Resume, LOCTEXT("Resume", "Resume Game")},
		{EFeatureMenuAction::ReturnToMenu, LOCTEXT("MainMenu", "Main Menu")}
	}};
}

FFeatureMenuView FFeatureMenuViews::VictoryMenu()
{
	return {LOCTEXT("Victory", "Victory"), false, false, false, {
		{EFeatureMenuAction::ReturnToMenu, LOCTEXT("Continue", "Continue")}
	}};
}

FFeatureMenuView FFeatureMenuViews::DefeatMenu()
{
	return {LOCTEXT("Defeat", "Defeat"), false, false, false, {
		{EFeatureMenuAction::Retry, LOCTEXT("Retry", "Retry")}
	}};
}

#undef LOCTEXT_NAMESPACE
