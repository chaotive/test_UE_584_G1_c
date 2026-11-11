#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FeatureBallPanel.generated.h"

class AFeatureGameState;
class UTextBlock;

/** Owns the ball diagnostics and their game-state observation inside the debug HUD. */
UCLASS()
class TESTFEATURES_API UFeatureBallPanel : public UUserWidget
{
	GENERATED_BODY()
protected:
	// Builds the ball diagnostics once.
	// This widget owns its tree.
	virtual void NativeOnInitialized() override;
	// Subscribes to bounce changes and synchronizes the initial display.
	// The world-owned state is observed weakly.
	virtual void NativeConstruct() override;
	// Removes the bounce subscription when the panel leaves the viewport.
	// Releases observation without owning the state.
	virtual void NativeDestruct() override;
private:
	// Creates the bounce-count label and its presentation.
	// Controls belong to the widget tree.
	void BuildLayout();
	// Displays the accumulated bounce total.
	void SetTotalBounces(int32 TotalBounces);
	// Displays the current saved level.
	void SetCurrentLevel(int32 CurrentLevel);
	// Displays the total reported by the game state.
	// UPROPERTY tracks a strong TObjectPtr to the tree-owned text widget for garbage collection.
	UPROPERTY()
	TObjectPtr<UTextBlock> TotalBouncesText;
	// Displays the saved level in the debug state panel.
	// UPROPERTY tracks a strong TObjectPtr to the tree-owned text widget for garbage collection.
	UPROPERTY()
	TObjectPtr<UTextBlock> CurrentLevelText;
	// Observes shared bounce changes without retaining the world actor.
	// TWeakObjectPtr detects actor destruction without extending its lifetime.
	TWeakObjectPtr<AFeatureGameState> ObservedState;
};
