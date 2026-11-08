#include "FeatureMenuActions.h"

FFeatureMenuActionDefinition FFeatureMenuActionDefinition::WithLabel(const FText& Caption) const
{
	return {Action, Caption};
}

FFeatureMenuActionDefinition FFeatureMenuSessionActions::NewGame()
{
	return {EFeatureMenuAction::NewGame, NSLOCTEXT("FeatureMenuViews", "NewGame", "New Game")};
}

FFeatureMenuActionDefinition FFeatureMenuSessionActions::Continue()
{
	return {EFeatureMenuAction::Continue, NSLOCTEXT("FeatureMenuViews", "ContinueLevelOne", "Continue on level 1")};
}

FFeatureMenuActionDefinition FFeatureMenuSessionActions::Resume()
{
	return {EFeatureMenuAction::Resume, NSLOCTEXT("FeatureMenuViews", "Resume", "Resume Game")};
}

FFeatureMenuActionDefinition FFeatureMenuSessionActions::Retry()
{
	return {EFeatureMenuAction::Retry, NSLOCTEXT("FeatureMenuViews", "Retry", "Retry")};
}

FFeatureMenuActionDefinition FFeatureMenuPersistenceActions::DeleteSave()
{
	return {EFeatureMenuAction::DeleteSave, NSLOCTEXT("FeatureMenuViews", "DeleteSave", "Delete Save")};
}

FFeatureMenuActionDefinition FFeatureMenuNavigationActions::ReturnToMenu()
{
	return {EFeatureMenuAction::ReturnToMenu, NSLOCTEXT("FeatureMenuViews", "MainMenu", "Main Menu")};
}

FFeatureMenuActionDefinition FFeatureMenuNavigationActions::Exit()
{
	return {EFeatureMenuAction::Exit, NSLOCTEXT("FeatureMenuViews", "Exit", "Exit")};
}
