#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TrainingDebugHUD.generated.h"

class UBorder;
class UTextBlock;
class ATrainingGameState;
class UTrainingSaveGameSubsystem;

/** A code-built debug overlay with live state and scrollable persistence diagnostics. */
UCLASS()
class UTrainingDebugHUD : public UUserWidget
{
	GENERATED_BODY()

public:

	// Formats the same persistence diagnostics for the HUD and blocking modal.
	// Saves is a read-only subsystem reference; the formatted FString is returned by value.
	static FString FormatSaveDiagnostics(const UTrainingSaveGameSubsystem& Saves);

protected:

	// Builds the overlay once when CreateWidget initializes this widget instance.
	// No pointer or reference parameters or return value; modifies this player-owned widget.
	virtual void NativeOnInitialized() override;

	// Connects this HUD to the shared count and displays its current value when shown.
	// No pointer or reference parameters or return value; caches a weak reference to the world-owned state.
	virtual void NativeConstruct() override;

	// Disconnects this HUD from state events when its underlying UI is removed.
	// No pointer or reference parameters or return value; releases the subscription without owning the state.
	virtual void NativeDestruct() override;

private:

	// Refreshes persistence details when an I/O phase or result changes.
	// Status is read-only and bBusy is by value; the widget reads the retained diagnostic history.
	void RefreshSaveStatus(const FString& Status, bool bBusy);

	// Displays the shared bounce total received from the state event.
	// TotalBounces is passed by value; no pointers or references are passed or returned.
	void SetTotalBounces(int32 TotalBounces);

	// Positions the debug panel in the top-left corner of the player's screen.
	// No pointer or reference parameters or return value; the widget tree owns the created controls.
	void BuildLayout();

	// Creates the panel background and its debug information rows.
	// Returns a raw pointer to a border owned by this widget's UWidgetTree without transferring ownership.
	UBorder* BuildPanel();

	// Creates a consistently styled text row for the debug panel.
	// Text is a read-only reference, FontSize is by value, and the raw return pointer refers to a tree-owned control.
	UTextBlock* CreateText(const FText& Text, int32 FontSize);

	// Retains the text row updated when the shared bounce count changes.
	// UPROPERTY tracks a strong TObjectPtr to the widget-tree-owned text control for garbage collection.
	UPROPERTY()
	TObjectPtr<UTextBlock> TotalBouncesText;

	// Remembers the observed state so the HUD can disconnect on removal or reconstruction.
	// TWeakObjectPtr observes the world-owned actor without keeping it alive and detects destruction during teardown.
	TWeakObjectPtr<ATrainingGameState> ObservedState;

	// Displays slot, physical desktop file path, and operation history including blocking phases.
	// UPROPERTY tracks a strong TObjectPtr to a widget-tree-owned text control.
	UPROPERTY()
	TObjectPtr<UTextBlock> SaveDiagnosticsText;

	// Observes the persistence diagnostic source without extending its lifetime.
	// TWeakObjectPtr detects subsystem destruction and does not prevent garbage collection.
	TWeakObjectPtr<UTrainingSaveGameSubsystem> ObservedSaves;
};
