#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TrainingBallPanel.generated.h"

class ATrainingGameState;
class UTextBlock;

/** Owns the ball diagnostics and their game-state observation inside the debug HUD. */
UCLASS()
class UTrainingBallPanel : public UUserWidget
{
	GENERATED_BODY()
protected:
	// Builds the ball diagnostics once.
	// No pointer or reference parameters or return value; this widget owns its tree.
	virtual void NativeOnInitialized() override;
	// Subscribes to bounce changes and synchronizes the initial display.
	// No pointer or reference parameters or return value; the world-owned state is observed weakly.
	virtual void NativeConstruct() override;
	// Removes the bounce subscription when the panel leaves the viewport.
	// No pointer or reference parameters or return value; releases observation without owning the state.
	virtual void NativeDestruct() override;
private:
	// Creates the bounce-count label and its presentation.
	// No pointer or reference parameters or return value; controls belong to the widget tree.
	void BuildLayout();
	// Displays the accumulated bounce total.
	// TotalBounces is passed by value without pointers or references.
	void SetTotalBounces(int32 TotalBounces);
	// Displays the total reported by the game state.
	// UPROPERTY tracks a strong TObjectPtr to the tree-owned text widget for garbage collection.
	UPROPERTY()
	TObjectPtr<UTextBlock> TotalBouncesText;
	// Observes shared bounce changes without retaining the world actor.
	// TWeakObjectPtr detects actor destruction without extending its lifetime.
	TWeakObjectPtr<ATrainingGameState> ObservedState;
};
