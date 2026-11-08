#pragma once

#include "Blueprint/UserWidget.h"
#include "Misc/Optional.h"
#include "FeatureSavedProgressPanel.generated.h"

class UFeatureSaveGameSubsystem;
class UTextBlock;
struct FFeatureGameProgress;

/** Owns saved-progress preview loading, summary presentation, and deletion observation. */
UCLASS()
class TESTFEATURES_API UFeatureSavedProgressPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_EVENT_TwoParams(UFeatureSavedProgressPanel, FPreviewChangedEvent, const TOptional<int32>&, bool);

	// Announces the saved level and slot availability for related menu actions.
	// The optional level is borrowed during broadcast and the bool is copied; native delegates store weak UObject bindings.
	FPreviewChangedEvent OnPreviewChanged;

	// Selects the full startup summary or the compact pause summary before presentation.
	// bDetailed is copied by value without pointers or references.
	void Configure(bool bDetailed);

	// Requests deletion through the observed persistence subsystem.
	// No pointer or reference arguments or return value; the game instance owns the subsystem.
	void DeleteSave();

	// Reports the level available from the most recently completed preview.
	// Returns a borrowed const optional reference; TOptional distinguishes an unreadable or absent preview from a saved level.
	const TOptional<int32>& GetSavedLevel() const;

	// Reports whether a physical save slot is available for deletion.
	// Returns bool by value without pointers or references.
	bool HasSaveSlot() const;

protected:
	// Builds the summary label once with the menu's existing typography.
	// No pointer or reference arguments; the widget tree owns the created label.
	virtual void NativeOnInitialized() override;

	// Observes deletion and starts a fresh preview when displayed.
	// No pointer or reference arguments; observes the game-instance-owned subsystem weakly.
	virtual void NativeConstruct() override;

	// Invalidates pending previews and removes deletion observation when detached.
	// No pointer or reference arguments; releases weak bindings without retaining the subsystem.
	virtual void NativeDestruct() override;

private:
	// Loads saved progress asynchronously without pausing gameplay for a menu preview.
	// No pointer or reference arguments; the callback observes this widget through a weak pointer.
	void RefreshPreview();

	// Builds the full or compact summary from a loaded snapshot.
	// Progress is a borrowed const reference and FText is returned by value without snapshot ownership.
	FText FormatProgress(const FFeatureGameProgress& Progress) const;

	// Updates the summary label and publishes the current action availability.
	// Summary is a borrowed const text reference; listeners borrow the optional level only during broadcast.
	void PublishPreview(const FText& Summary);

	// Clears the displayed preview after deletion or reports a failed attempt.
	// bSucceeded is copied by value without pointers or references.
	void HandleSaveDeleted(bool bSucceeded);

	// Retains the label displaying the current preview or operation result.
	// UPROPERTY tracks the strong widget-tree-owned text pointer for garbage collection.
	UPROPERTY()
	TObjectPtr<UTextBlock> SummaryText;

	// Observes save-deletion events while the panel is displayed.
	// TWeakObjectPtr does not extend the lifetime of the game-instance-owned subsystem.
	TWeakObjectPtr<UFeatureSaveGameSubsystem> ObservedSaves;

	// Stores the level used by Continue when a preview successfully loads.
	// TOptional owns an int32 value and represents absent or unreadable progress without a duplicated default level.
	TOptional<int32> SavedLevel;

	// Invalidates callbacks after deletion, refresh, or removal from the viewport.
	// uint32 stores a generation token by value without pointer ownership.
	uint32 PreviewGeneration = 0;

	// Selects startup details instead of the pause menu's saved total alone.
	// bool stores presentation configuration by value without pointer ownership.
	bool bDetailedSummary = false;

	// Distinguishes an existing slot from a readable saved level.
	// bool stores preview state by value so unreadable saves remain deletable.
	bool bHasSaveSlot = false;
};
