#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TrainingSaveGameSubsystem.generated.h"

/** Coordinates synchronous persistence and publishes diagnostic state without rendering UI. */
UCLASS()
class UTrainingSaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	DECLARE_EVENT_TwoParams(UTrainingSaveGameSubsystem, FStatusChangedEvent, const FString&, bool);

	// Announces operation transitions before I/O starts and after it completes.
	// The native event borrows a const string reference and passes busy state by value; listeners are not owned.
	FStatusChangedEvent OnStatusChanged;

	DECLARE_EVENT(UTrainingSaveGameSubsystem, FBeforeIOEvent);

	// Lets UI listeners finish presenting the published phase before disk access begins.
	// Native event has no pointer or reference parameters; the subsystem does not know which listeners render UI.
	FBeforeIOEvent OnBeforeIO;

	// Exposes all transitions so a HUD created after loading still shows earlier phases.
	// Returns a const TArray reference without copying or allowing callers to modify the history.
	const TArray<FString>& GetStatusHistory() const;

	// Identifies the save slot for diagnostic consumers.
	// Returns a const FString reference owned by the storage helper without ownership transfer.
	const FString& GetSlotName() const;

	// Identifies the desktop save file being read or written.
	// Returns an absolute FString path by value without pointers or references.
	FString GetSaveFilePath() const;

	// Restores the saved total while a modal loading screen blocks interaction.
	// Returns int32 by value without pointer or reference ownership transfer.
	int32 LoadTotalBounces();

	// Completes a save before returning control to level teardown or shutdown.
	// TotalBounces and the bool success result are passed by value without live actor references.
	bool SaveTotalBounces(int32 TotalBounces);

private:

	// Records a transition and notifies diagnostic consumers synchronously.
	// Status is read-only and bBusy is by value; the history retains a string copy.
	void PublishStatus(const FString& Status, bool bBusy);

	// Retains startup and operation results until the game instance ends.
	// TArray owns copied FString values; no live actors or UI objects are retained.
	TArray<FString> StatusHistory;
};
