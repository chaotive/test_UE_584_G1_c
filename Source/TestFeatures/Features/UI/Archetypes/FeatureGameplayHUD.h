#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FeatureGameplayHUD.generated.h"

class AFeatureGameState;
class UFeatureGameplaySession;
class UTextBlock;
struct FFeatureGameplaySessionRecord;

/** Displays the current session's progress toward its win and loss thresholds. */
UCLASS()
class TESTFEATURES_API UFeatureGameplayHUD : public UUserWidget
{
	GENERATED_BODY()

protected:
	// Builds the objective display once for this widget instance.
	// No pointer or reference parameters or return value; the widget tree owns its controls.
	virtual void NativeOnInitialized() override;

	// Observes session statistics and synchronizes the current display.
	// No pointer or reference parameters or return value; the world owns the observed state and session.
	virtual void NativeConstruct() override;

	// Stops observing session statistics when the widget leaves the viewport.
	// No pointer or reference parameters or return value; weak observations do not own world objects.
	virtual void NativeDestruct() override;

private:
	// Constructs the objective panel and its labels.
	// No pointer or reference parameters or return value; controls belong to the widget tree.
	void BuildLayout();

	// Updates both objective counts from the active session record.
	// Stats is a borrowed read-only reference to state-owned value data.
	void UpdateStats(const FFeatureGameplaySessionRecord& Stats);

	// Displays the current saved level.
	void SetCurrentLevel(int32 CurrentLevel);

	// Displays the current progression level above the objective counts.
	// UPROPERTY tracks a strong TObjectPtr to the tree-owned text widget.
	UPROPERTY()
	TObjectPtr<UTextBlock> CurrentLevelText;

	// Displays player contacts and the victory threshold.
	// UPROPERTY tracks a strong TObjectPtr to the tree-owned text widget.
	UPROPERTY()
	TObjectPtr<UTextBlock> PlayerContactsText;

	// Displays non-player bounces and the defeat threshold.
	// UPROPERTY tracks a strong TObjectPtr to the tree-owned text widget.
	UPROPERTY()
	TObjectPtr<UTextBlock> BallBouncesText;

	// Observes session statistics without retaining the world-owned game state.
	// TWeakObjectPtr tracks actor lifetime without taking ownership.
	TWeakObjectPtr<AFeatureGameState> ObservedState;

	// Reads the session's configured thresholds without retaining the world subsystem.
	// TWeakObjectPtr tracks subsystem lifetime without taking ownership.
	TWeakObjectPtr<UFeatureGameplaySession> ObservedSession;
};
