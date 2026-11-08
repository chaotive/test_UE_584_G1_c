#pragma once

#include "Blueprint/UserWidget.h"
#include "Features/UI/Views/FeatureMenuViews.h"
#include "Features/UI/Controllers/FeatureMenuActions.h"
#include "Misc/Optional.h"
#include "FeatureMainMenu.generated.h"

class UButton;
class UGameInstance;
class UFeatureSavedProgressPanel;
class UTexture2D;
class UVerticalBox;

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
	// Binds save-related buttons to the child panel's current preview.
	// No pointer or reference parameters; the menu owns its child widget and weak native event binding.
	virtual void NativeConstruct() override;

	// Removes the preview binding when the menu is detached.
	// No pointer or reference parameters or return value; releases the weak native event binding.
	virtual void NativeDestruct() override;

	// Routes Escape to Resume or Exit for the pause and initial menus.
	// The geometry and key event are borrowed const references; FReply is returned by value.
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& KeyEvent) override;

private:
	// Builds the menu for the selected flow.
	// Flow is passed by value without pointer or reference ownership.
	void BuildMenu(EFeatureGameFlow Flow);

	// Builds the shared logo, title, subtitle, and optional view heading.
	// Rows and View are borrowed references; the widget tree owns the created controls.
	void BuildHeader(UVerticalBox& Rows, const FFeatureMenuView& View);

	// Adds the saved game summary below the menu actions.
	// Rows is borrowed and the initial-menu flag is copied; the child widget owns preview loading and presentation.
	void AddSaveSummary(UVerticalBox& Rows, bool bInitialMenu);

	// Retains the startup Continue button while its saved level is read.
	// UPROPERTY tracks a strong widget-tree-owned button pointer for garbage collection.
	UPROPERTY()
	TObjectPtr<UButton> ContinueButton;

	// Retains the startup Delete Save button for refresh after deletion.
	// UPROPERTY tracks a strong widget-tree-owned button pointer for garbage collection.
	UPROPERTY()
	TObjectPtr<UButton> DeleteSaveButton;

	// Retains the child that owns saved-progress loading and summary presentation.
	// UPROPERTY tracks the strong child-widget pointer for garbage collection.
	UPROPERTY()
	TObjectPtr<UFeatureSavedProgressPanel> SavedProgressPanel;

	// Retains the declarative Continue caption used when no readable level is available.
	// FText owns a localized value copied from the view definition without widget ownership.
	FText DefaultContinueLabel;

	// Keeps the generated logo texture available while this menu is displayed.
	// UPROPERTY tracks the strong TObjectPtr to the transient texture for garbage collection.
	UPROPERTY()
	TObjectPtr<UTexture2D> LogoTexture;

	// Identifies whether this menu supports Escape to resume.
	// bool stores the input behavior by value.
	bool bPauseMenu = false;

	// Identifies whether Escape exits from this menu.
	// bool stores the input behavior by value without pointer ownership.
	bool bExitOnEscape = false;

	// Adds a labeled action button to the menu stack.
	// Rows and Label are borrowed references; the returned button is owned by the widget tree.
	UButton* AddAction(UVerticalBox& Rows, const FText& Label);

	// Creates a view-defined button and connects its menu action.
	// Rows and Item are borrowed references; the widget tree owns the button.
	void AddViewAction(UVerticalBox& Rows, const FFeatureMenuActionDefinition& Item);

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

	// Publishes Delete Save so the action controller can clear the slot.
	// No pointer or reference parameters or return value; bound to a widget-owned button.
	UFUNCTION()
	void HandleDeleteSave();

	// Updates Continue and Delete Save from the child's narrow preview state.
	// SavedLevel is a borrowed const optional reference and bHasSave is copied; no storage data is retained.
	void UpdateSaveActions(const TOptional<int32>& SavedLevel, bool bHasSave);

	// Publishes the Exit selection.
	// No pointer or reference parameters or return value; bound to a widget-owned button.
	UFUNCTION()
	void HandleExit();
};
