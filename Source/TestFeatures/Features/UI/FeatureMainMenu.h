#pragma once

#include "Blueprint/UserWidget.h"
#include "FeatureMainMenu.generated.h"

class UButton;
class UGameInstance;
class UVerticalBox;

enum class EFeatureGameFlow : uint8;

enum class EFeatureMenuAction : uint8
{
	NewGame,
	Continue,
	Resume,
	ReturnToMenu,
	Retry,
	Exit
};

/** Presents the actions available in a menu flow state. */
UCLASS()
class TESTFEATURES_API UFeatureMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_EVENT_OneParam(UFeatureMainMenu, FActionEvent, EFeatureMenuAction);

	// Publishes the action selected in the menu.
	// The native multicast event passes the enum by value without pointer ownership.
	FActionEvent OnAction;

	// Creates and presents the fullscreen menu for the current flow state.
	// GameInstance is borrowed and Flow is passed by value. The caller and viewport retain the returned widget.
	static UFeatureMainMenu* Present(UGameInstance& GameInstance, EFeatureGameFlow Flow);

	// Removes the menu and returns keyboard and mouse routing to gameplay.
	// No pointer or reference parameters or return value; operates on the viewport hosting this widget.
	void Dismiss();

protected:
	// Treats Escape as Resume while the pause menu or one of its buttons has focus.
	// The geometry and key event are borrowed const references; FReply is returned by value.
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& KeyEvent) override;

private:
	// Builds the menu for the selected flow.
	// Flow is passed by value without pointer or reference ownership.
	void BuildMenu(EFeatureGameFlow Flow);

	// Adds the saved game summary below the menu actions.
	// Rows is a borrowed widget reference and bHasSave is passed by value.
	void AddSaveSummary(UVerticalBox& Rows, bool bHasSave);

	// Retains the startup Continue button while its saved level is read.
	// UPROPERTY tracks a strong widget-tree-owned button pointer for garbage collection.
	UPROPERTY()
	TObjectPtr<UButton> ContinueButton;

	// Identifies whether this menu supports Escape to resume.
	// bool stores the input behavior by value.
	bool bPauseMenu = false;

	// Adds a labeled action button to the menu stack.
	// Rows and Label are borrowed references; the returned button is owned by the widget tree.
	UButton* AddAction(UVerticalBox& Rows, const FText& Label);

	// Publishes the New Game selection.
	// No pointer or reference parameters or return value; bound to a widget-owned button.
	UFUNCTION()
	void HandleNewGame();

	// Publishes the Continue selection.
	// No pointer or reference parameters or return value; bound to a widget-owned button.
	UFUNCTION()
	void HandleContinue();

	// Publishes the Resume selection.
	// No pointer or reference parameters or return value; bound to a widget-owned button.
	UFUNCTION()
	void HandleResume();

	// Publishes the Return to Main Menu selection.
	// No pointer or reference parameters or return value; bound to a widget-owned button.
	UFUNCTION()
	void HandleReturnToMenu();

	// Publishes the request to retry a defeated session.
	// No pointer or reference parameters or return value; bound to a widget-owned button.
	UFUNCTION()
	void HandleRetry();

	// Publishes the Exit selection.
	// No pointer or reference parameters or return value; bound to a widget-owned button.
	UFUNCTION()
	void HandleExit();
};
