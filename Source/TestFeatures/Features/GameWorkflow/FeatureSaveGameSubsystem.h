#pragma once
#include "CoreMinimal.h"
#include "FeatureSaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FeatureSaveGameSubsystem.generated.h"

/** Async persistence state machine; gameplay pauses while HUD-managed previews and I/O run. */
UCLASS()
class TESTFEATURES_API UFeatureSaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	DECLARE_EVENT_TwoParams(UFeatureSaveGameSubsystem, FStatusChangedEvent, const FString&, bool);
	DECLARE_EVENT_OneParam(UFeatureSaveGameSubsystem, FLoadedEvent, FFeatureGameProgress&);
	DECLARE_EVENT_OneParam(UFeatureSaveGameSubsystem, FSaveDeletedEvent, bool);
	// Announces preview, disk activity, completion, and resume transitions.
	// Native multicast event borrows a string and passes bool by value without owning listeners.
	FStatusChangedEvent OnStatusChanged;
	// Delivers restored data before gameplay resumes.
	// Native event borrows a mutable snapshot so gameplay initialization updates a queued save-after-load.
	FLoadedEvent OnLoaded;
	// Reports the outcome of a request to clear the save slot.
	// The native event passes the success bool by value without owning listeners.
	FSaveDeletedEvent OnSaveDeleted;

	// Reports whether a save exists for a Continue request.
	// Returns bool by value without pointers or references.
	bool HasSave() const;
	// Deletes the slot and announces whether the operation succeeded.
	// Returns bool by value without pointers or references.
	bool DeleteSave();

	// Requests a paused, in-game load with HUD previews.
	// No pointer or reference parameters or return value; callbacks use this game-instance subsystem.
	void RequestLoad();
	// Requests an in-game save, optionally followed by a successful orderly quit.
	// Progress is a borrowed read-only value snapshot and bQuitAfter is passed by value.
	void RequestSave(const FFeatureGameProgress& Progress, bool bQuitAfter = false);
	// Starts disk access after the HUD's nonblocking pre-operation hold.
	// No pointer or reference parameters or return value; starts one pending operation.
	void BeginPendingIO();
	// Resumes gameplay or quits after the HUD's nonblocking result hold.
	// No pointer or reference parameters or return value; applies the operation outcome.
	void FinishOperation();
	// Invalidates callbacks when the owning level is forcibly torn down.
	// No pointer or reference parameters or return value; advances a value generation token.
	void CancelOperation();
	// Reports whether gameplay is currently gated by persistence.
	// Returns bool by value without pointers or references.
	bool IsBusy() const { return bBusy; }
	// Reports whether the pending disk call is waiting for its preview.
	// Returns bool by value without pointers or references.
	bool IsWaitingForIO() const { return bWaitingForIO; }
	// Reports whether the result preview may now end the operation.
	// Returns bool by value without pointers or references.
	bool IsIOComplete() const { return bIOComplete; }
	// Reports whether an orderly quit already persisted the current snapshot.
	// Returns bool by value without pointers or references.
	bool HasSavedForQuit() const { return bSavedForQuit; }
	// Reads retained diagnostic transitions.
	// Returns a const array reference without copying or transferring ownership.
	const TArray<FString>& GetStatusHistory() const;
	// Identifies the storage slot.
	// Returns a const string reference owned by storage.
	const FString& GetSlotName() const;
	// Identifies the desktop save file.
	// Returns the absolute path by value without object references.
	FString GetSaveFilePath() const;
private:
	// Pauses the current world and announces a pending operation.
	// Progress is borrowed read-only; bLoad and bQuitAfter are values without ownership transfer.
	void StartOperation(bool bLoad, const FFeatureGameProgress& Progress, bool bQuitAfter);
	// Records a diagnostic transition for HUD observers.
	// Status is borrowed read-only and copied into owned history.
	void PublishStatus(const FString& Status);
	// Retains diagnostics across levels.
	// TArray owns FString values and no actor pointers.
	TArray<FString> StatusHistory;
	// Tracks the world whose pause state this operation owns.
	// TWeakObjectPtr observes world lifetime without retaining a destroyed level.
	TWeakObjectPtr<UWorld> OperationWorld;
	// Identifies the current callback generation across level teardown.
	// uint32 is a value token without pointer ownership.
	uint32 OperationId = 0;
	// Holds the requested or restored progress, including session records.
	// The reflected struct owns a value snapshot without live actor references.
	FFeatureGameProgress PendingProgress;
	// Indicates a persistence gate is active.
	// bool is per-instance value state without pointers.
	bool bBusy = false;
	// Indicates that the HUD preview must run before disk access.
	// bool is per-instance value state without pointers.
	bool bWaitingForIO = false;
	// Indicates a completed operation waiting for its result preview.
	// bool is per-instance value state without pointers.
	bool bIOComplete = false;
	// Selects loading rather than saving.
	// bool is per-instance value state without pointers.
	bool bLoading = false;
	// Remembers the operation's success.
	// bool is per-instance value state without pointers.
	bool bSucceeded = false;
	// Requests quit only after a successful save and preview.
	// bool is per-instance value state without pointers.
	bool bQuitRequested = false;
	// Preserves a pause that existed before this operation.
	// bool is per-instance value state without pointers.
	bool bWasPaused = false;
	// Suppresses redundant shutdown saving after the gated quit path.
	// bool is per-instance value state without pointers.
	bool bSavedForQuit = false;
};
