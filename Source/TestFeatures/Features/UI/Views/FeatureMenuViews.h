#pragma once

#include "CoreMinimal.h"
#include "Features/GameWorkflow/FeatureGameFlow.h"

enum class EFeatureMenuAction : uint8
{
	NewGame,
	Continue,
	Resume,
	ReturnToMenu,
	Retry,
	DeleteSave,
	Exit
};

/** Describes one menu button and the action it publishes. */
struct FFeatureMenuItem
{
	// Selects the operation represented by this button.
	// The enum is stored by value without pointer or reference ownership.
	EFeatureMenuAction Action;

	// Supplies the localized button caption.
	// FText stores a localized value without widget ownership.
	FText Label;
};

/** Describes the title, input behavior, and ordered controls for one menu. */
struct FFeatureMenuView
{
	// Supplies the heading; an empty value selects the configured project title.
	// FText stores a localized value without widget ownership.
	FText Heading;

	// Enables Escape to resume gameplay for the pause view.
	// bool is stored by value without pointer ownership.
	bool bPauseMenu;

	// Makes Escape invoke Exit on the initial menu.
	// bool is stored by value without pointer ownership.
	bool bExitOnEscape;

	// Requests the saved-game summary below the action list.
	// bool is stored by value without pointer ownership.
	bool bShowSaveSummary;

	// Lists the buttons in their presentation order.
	// TArray owns the item values and their localized labels.
	TArray<FFeatureMenuItem> Items;
};

/** Supplies declarative menu definitions independently of widget construction. */
class TESTFEATURES_API FFeatureMenuViews
{
public:
	// Selects the view corresponding to the current game flow.
	// Flow and the returned view are values; no pointers or references are transferred.
	static FFeatureMenuView ForFlow(EFeatureGameFlow Flow);

private:
	// Defines the startup menu and its save controls.
	// Returns an owned view value without pointer or reference parameters.
	static FFeatureMenuView InitialMenu();

	// Defines the paused-game menu and its input behavior.
	// Returns an owned view value without pointer or reference parameters.
	static FFeatureMenuView PauseMenu();

	// Defines the victory result actions.
	// Returns an owned view value without pointer or reference parameters.
	static FFeatureMenuView VictoryMenu();

	// Defines the defeat result actions.
	// Returns an owned view value without pointer or reference parameters.
	static FFeatureMenuView DefeatMenu();
};
