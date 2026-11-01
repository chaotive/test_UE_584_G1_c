#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TrainingDebugHUD.generated.h"

class UBorder;
class UTextBlock;
class ATrainingGameState;
class UTrainingSaveGamePanel;

/** Composes debug sections and displays the shared bounce count. */
UCLASS()
class UTrainingDebugHUD : public UUserWidget
{
	GENERATED_BODY()
protected:
	// Builds the outer debug overlay once.
	// No pointer or reference parameters or return value; this widget owns its tree.
	virtual void NativeOnInitialized() override;
	// Connects the bounce display and enables interactive debug input.
	// No pointer or reference parameters or return value; the game-state reference is weak.
	virtual void NativeConstruct() override;
	// Disconnects the bounce display when the overlay is removed.
	// No pointer or reference parameters or return value; releases observation without owning the state.
	virtual void NativeDestruct() override;
private:
	// Updates the bounce-count label.
	// Count is passed by value without pointers or references.
	void SetTotalBounces(int32 TotalBounces);
	// Positions the composed debug panel in the player's viewport.
	// No pointer or reference parameters or return value; controls belong to the widget tree.
	void BuildLayout();
	// Builds the title, bounce section, and specialized persistence child widget.
	// Returns a raw pointer to a tree-owned border without ownership transfer.
	UBorder* BuildPanel();
	// Creates a styled debug label.
	// Text is read-only, size is by value, and the returned raw pointer refers to a tree-owned control.
	UTextBlock* CreateText(const FText& Text, int32 FontSize);
	// Displays the shared bounce total.
	// UPROPERTY tracks a strong TObjectPtr to a tree-owned text widget.
	UPROPERTY()
	TObjectPtr<UTextBlock> TotalBouncesText;
	// Observes shared bounce changes without retaining the world actor.
	// TWeakObjectPtr detects actor destruction without extending its lifetime.
	TWeakObjectPtr<ATrainingGameState> ObservedState;
	// Composes all save/load UI inside the main debug HUD.
	// UPROPERTY retains a strong TObjectPtr to the child user widget for garbage collection.
	UPROPERTY()
	TObjectPtr<UTrainingSaveGamePanel> SavePanel;
};
