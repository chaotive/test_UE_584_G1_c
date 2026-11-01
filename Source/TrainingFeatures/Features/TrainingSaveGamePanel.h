#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Containers/Ticker.h"
#include "TrainingSaveGamePanel.generated.h"

class ATrainingGameState;
class UTrainingSaveGameSubsystem;
class UHorizontalBox;
class UTextBlock;

/** Specialized in-game persistence UI, composed inside the debug HUD. */
UCLASS()
class TRAININGFEATURES_API UTrainingSaveGamePanel : public UUserWidget
{
	GENERATED_BODY()
protected:
	// Builds the persistence controls and diagnostic section once.
	// No pointer or reference parameters or return value; this widget owns its tree.
	virtual void NativeOnInitialized() override;
	// Connects save diagnostics, real-time previews, and orderly window close handling.
	// No pointer or reference parameters or return value; cached world references are weak.
	virtual void NativeConstruct() override;
	// Removes bindings and pending preview callbacks when this section is removed.
	// No pointer or reference parameters or return value; releases observations without destroying systems.
	virtual void NativeDestruct() override;
private:
	// Composes the persistence section from controls and a scrollable history.
	// No pointer or reference parameters or return value; controls belong to the widget tree.
	void BuildLayout();
	// Creates save and save-and-quit controls.
	// Returns a raw pointer to a tree-owned control without ownership transfer.
	UHorizontalBox* BuildActions();
	// Creates a consistently styled diagnostic or button label.
	// Text is read-only, size is by value, and the returned raw pointer refers to a tree-owned widget.
	UTextBlock* CreateText(const FText& Text, int32 FontSize);
	// Formats slot, path, phase, and retained operation history.
	// Saves is a read-only reference; the formatted string is returned by value.
	static FString FormatSaveDiagnostics(const UTrainingSaveGameSubsystem& Saves);
	// Refreshes diagnostics and schedules any required one-second preview.
	// Status is read-only and busy is by value; event data is not retained by reference.
	void RefreshSaveStatus(const FString& Status, bool bBusy);
	// Advances a pending operation after its nonblocking real-time preview.
	// DeltaSeconds and the one-shot bool result are by value without pointer ownership.
	bool HandlePreviewElapsed(float DeltaSeconds);
	// Requests saving without leaving the current game.
	// No pointer or reference parameters or return value; acts on the weakly observed state.
	UFUNCTION()
	void HandleSaveClicked();
	// Requests saving followed by a successful orderly quit.
	// No pointer or reference parameters or return value; acts on the weakly observed state.
	UFUNCTION()
	void HandleSaveAndQuitClicked();
	// Gates native game-window close through the save-and-quit flow.
	// Returns bool by value; false rejects immediate window close without owning it.
	bool HandleWindowCloseRequested();
	// Displays the save slot, physical desktop path, phase, and history.
	// UPROPERTY tracks a strong TObjectPtr to a widget-tree-owned text block.
	UPROPERTY()
	TObjectPtr<UTextBlock> SaveDiagnosticsText;
	// Observes the state that supplies the save snapshot.
	// TWeakObjectPtr observes a world-owned actor without extending its lifetime.
	TWeakObjectPtr<ATrainingGameState> ObservedState;
	// Observes operation state across asynchronous persistence phases.
	// TWeakObjectPtr does not retain the game-instance-owned subsystem.
	TWeakObjectPtr<UTrainingSaveGameSubsystem> ObservedSaves;
	// Tracks the pending pause-independent UI preview callback.
	// FTSTicker::FDelegateHandle is a weak ticker-entry handle removed on widget teardown.
	FTSTicker::FDelegateHandle PreviewTicker;
};
