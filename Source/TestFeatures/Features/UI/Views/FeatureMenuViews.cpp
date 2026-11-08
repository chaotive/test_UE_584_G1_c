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
	View.Items.Add(FFeatureMenuNavigationActions::Exit());
	return View;
}

FFeatureMenuView FFeatureMenuViews::InitialMenu()
{
	return {FText::GetEmpty(), false, true, true, {
		FFeatureMenuSessionActions::NewGame(),
		FFeatureMenuSessionActions::Continue(),
		FFeatureMenuPersistenceActions::DeleteSave()
	}};
}

FFeatureMenuView FFeatureMenuViews::PauseMenu()
{
	return {LOCTEXT("Paused", "Paused"), true, false, true, {
		FFeatureMenuSessionActions::Resume(),
		FFeatureMenuNavigationActions::ReturnToMenu()
	}};
}

FFeatureMenuView FFeatureMenuViews::VictoryMenu()
{
	return {LOCTEXT("Victory", "Victory"), false, false, false, {
		FFeatureMenuSessionActions::Retry().WithLabel(LOCTEXT("Continue", "Continue"))
	}};
}

FFeatureMenuView FFeatureMenuViews::DefeatMenu()
{
	return {LOCTEXT("Defeat", "Defeat"), false, false, false, {
		FFeatureMenuSessionActions::Retry()
	}};
}

#undef LOCTEXT_NAMESPACE
