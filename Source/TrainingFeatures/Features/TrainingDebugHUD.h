#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TrainingDebugHUD.generated.h"

class UBorder;
class UTextBlock;
class UTrainingBallPanel;
class UTrainingSaveGamePanel;

/** Composes specialized debug panels and manages their shared presentation. */
UCLASS()
class TRAININGFEATURES_API UTrainingDebugHUD : public UUserWidget
{
	GENERATED_BODY()
protected:
	// Builds the outer debug overlay once.
	// No pointer or reference parameters or return value; this widget owns its tree.
	virtual void NativeOnInitialized() override;
	// Enables interactive input for the composed debug panels.
	// No pointer or reference parameters or return value; the owning player manages input.
	virtual void NativeConstruct() override;
private:
	// Positions the composed debug panel in the player's viewport.
	// No pointer or reference parameters or return value; controls belong to the widget tree.
	void BuildLayout();
	// Builds the shared heading and composes the specialized child panels.
	// Returns a raw pointer to a tree-owned border without ownership transfer.
	UBorder* BuildPanel();
	// Creates a styled debug label.
	// Text is read-only, size is by value, and the returned raw pointer refers to a tree-owned control.
	UTextBlock* CreateText(const FText& Text, int32 FontSize);
	// Composes the ball diagnostics inside the main debug HUD.
	// UPROPERTY retains a strong TObjectPtr to the child user widget for garbage collection.
	UPROPERTY()
	TObjectPtr<UTrainingBallPanel> BallPanel;
	// Composes all save/load UI inside the main debug HUD.
	// UPROPERTY retains a strong TObjectPtr to the child user widget for garbage collection.
	UPROPERTY()
	TObjectPtr<UTrainingSaveGamePanel> SavePanel;
};
