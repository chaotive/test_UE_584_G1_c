#pragma once

#include "CoreMinimal.h"

/** Identifies UI operations independently of their presentation. */
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

/** Supplies an operation identity and its default localized caption. */
struct TESTFEATURES_API FFeatureMenuActionDefinition
{
	// Selects the operation published by a control.
	EFeatureMenuAction Action;

	// Supplies the localized caption, which a view may override.
	FText Label;

	// Copies the definition with a context-specific caption.
	// Borrows a const FText reference and returns an owned value without widget pointers.
	FFeatureMenuActionDefinition WithLabel(const FText& Caption) const;
};

/** Defines session operations for any UI view. */
class TESTFEATURES_API FFeatureMenuSessionActions
{
public:
	// Starts a new game.
	static FFeatureMenuActionDefinition NewGame();

	// Starts the saved game.
	static FFeatureMenuActionDefinition Continue();

	// Resumes the paused session.
	static FFeatureMenuActionDefinition Resume();

	// Restarts at the saved level.
	static FFeatureMenuActionDefinition Retry();
};

/** Defines persistence operations for any UI view. */
class TESTFEATURES_API FFeatureMenuPersistenceActions
{
public:
	// Deletes saved progress.
	static FFeatureMenuActionDefinition DeleteSave();
};

/** Defines navigation operations for any UI view. */
class TESTFEATURES_API FFeatureMenuNavigationActions
{
public:
	// Returns to the startup menu.
	static FFeatureMenuActionDefinition ReturnToMenu();

	// Exits the game through the current workflow.
	static FFeatureMenuActionDefinition Exit();
};
