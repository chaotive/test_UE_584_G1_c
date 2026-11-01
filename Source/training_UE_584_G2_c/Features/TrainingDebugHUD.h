#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TrainingDebugHUD.generated.h"

class UBorder;
class UTextBlock;

/** A non-interactive debug overlay built entirely in C++ without a Widget Blueprint. */
UCLASS()
class UTrainingDebugHUD : public UUserWidget
{
	GENERATED_BODY()

protected:

	// Builds the overlay once when CreateWidget initializes this widget instance.
	// No pointer or reference parameters or return value; modifies this player-owned widget.
	virtual void NativeOnInitialized() override;

private:

	// Positions the debug panel in the top-left corner of the player's screen.
	// No pointer or reference parameters or return value; the widget tree owns the created controls.
	void BuildLayout();

	// Creates the panel background and its debug information rows.
	// Returns a raw pointer to a border owned by this widget's UWidgetTree without transferring ownership.
	UBorder* BuildPanel();

	// Creates a consistently styled text row for the debug panel.
	// Text is a read-only reference, FontSize is by value, and the raw return pointer refers to a tree-owned control.
	UTextBlock* CreateText(const FText& Text, int32 FontSize);
};
