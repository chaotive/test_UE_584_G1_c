#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TrainingSaveGameUI.generated.h"

class UTrainingSaveGameSubsystem;
class SWindow;

/** Owns all modal presentation for persistence; observes operation events rather than performing I/O. */
UCLASS()
class UTrainingSaveGameUI : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	// Connects UI observation before a level can start loading its save.
	// Collection is a mutable reference used to initialize the persistence subsystem dependency first.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Disconnects observation and closes any remaining modal at game-instance teardown.
	// No pointer or reference parameters or return value; releases this subsystem's UI resources.
	virtual void Deinitialize() override;

private:

	// Opens or closes the blocking presentation according to the operation transition.
	// Status is a read-only event reference and bBusy is passed by value; neither is retained by reference.
	void HandleStatusChanged(const FString& Status, bool bBusy);

	// Builds and paints the persistence diagnostic modal before the operation starts.
	// No pointer or reference parameters or return value; retains a shared Slate window reference.
	void ShowModal();

	// Paints all updated UI after status listeners finish and before synchronous I/O starts.
	// No pointer or reference parameters or return value; synchronizes Slate rendering only.
	void PaintPendingUI();

	// Requests removal of the modal after completion or teardown.
	// No pointer or reference parameters or return value; releases the shared window reference.
	void CloseModal();

	// Observes persistence transitions for the entire game instance lifetime.
	// UPROPERTY tracks a strong TObjectPtr to a game-instance-owned subsystem for garbage collection.
	UPROPERTY()
	TObjectPtr<UTrainingSaveGameSubsystem> SaveSubsystem;

	// Retains the modal window while a blocking operation is in progress.
	// TSharedPtr manages a non-UObject Slate window through reference counting rather than UPROPERTY.
	TSharedPtr<SWindow> Modal;
};
