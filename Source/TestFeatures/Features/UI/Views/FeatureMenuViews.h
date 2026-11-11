#pragma once

#include "CoreMinimal.h"
#include "Features/UI/Controllers/FeatureMenuActions.h"
#include "Features/GameWorkflow/FeatureGameFlow.h"

/** Describes the title, input behavior, and ordered controls for one menu. */
struct FFeatureMenuView
{
	// Supplies the heading; an empty value selects the configured project title.
	FText Heading;

	// Enables Escape to resume gameplay for the pause view.
	bool bPauseMenu;

	// Makes Escape invoke Exit on the initial menu.
	bool bExitOnEscape;

	// Requests the saved-game summary below the action list.
	bool bShowSaveSummary;

	// Lists the buttons in their presentation order.
	// TArray owns the item values and their localized labels.
	TArray<FFeatureMenuActionDefinition> Items;
};

/** Supplies declarative menu definitions independently of widget construction. */
class TESTFEATURES_API FFeatureMenuViews
{
public:
	// Selects the view corresponding to the current game flow.
	static FFeatureMenuView ForFlow(EFeatureGameFlow Flow);

private:
	// Defines the startup menu and its save controls.
	static FFeatureMenuView InitialMenu();

	// Defines the paused-game menu and its input behavior.
	static FFeatureMenuView PauseMenu();

	// Defines the victory result actions.
	static FFeatureMenuView VictoryMenu();

	// Defines the defeat result actions.
	static FFeatureMenuView DefeatMenu();
};
