#include "FeatureSaveGameSubsystem.h"
#include "Features_NonUE/FeatureSaveGameStorage.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TestFeatures.h"

bool UFeatureSaveGameSubsystem::HasSave() const
{
	return FFeatureSaveGameStorage::HasSave();
}

bool UFeatureSaveGameSubsystem::DeleteSave()
{
	const bool bDeleted = FFeatureSaveGameStorage::DeleteSave();
	PublishStatus(bDeleted ? TEXT("Save slot cleared") : TEXT("Failed to delete save slot"));
	OnSaveDeleted.Broadcast(bDeleted);
	return bDeleted;
}

void UFeatureSaveGameSubsystem::RequestLoad()
{
	StartOperation(true, FFeatureGameProgress(), false);
}
void UFeatureSaveGameSubsystem::RequestSave(const FFeatureGameProgress& Progress, bool bQuitAfter)
{
	StartOperation(false, Progress, bQuitAfter);
}
void UFeatureSaveGameSubsystem::StartOperation(bool bLoad, const FFeatureGameProgress& Progress, bool bQuitAfter)
{
	// Serialize requests; repeated clicks or close requests must not start competing writes.
	if (bBusy)
	{
		bQuitRequested |= bQuitAfter;
		return;
	}
	++OperationId;
	OperationWorld = GetWorld();
	bWasPaused = UGameplayStatics::IsGamePaused(GetWorld());
	UGameplayStatics::SetGamePaused(GetWorld(), true);
	PendingProgress = Progress;
	bLoading = bLoad;
	bQuitRequested = bQuitAfter;
	bBusy = true;
	bSucceeded = false;
	BeginPendingIO();
}
void UFeatureSaveGameSubsystem::BeginPendingIO()
{
	PublishStatus(bLoading ? TEXT("Loading: disk I/O (game paused)") : TEXT("Saving: disk I/O (game paused)"));
	const uint32 RequestId = OperationId;
	TWeakObjectPtr<UFeatureSaveGameSubsystem> WeakThis(this);
	if (bLoading)
	{
		FFeatureSaveGameStorage::LoadAsync([WeakThis, RequestId](TOptional<FFeatureGameProgress> SavedProgress, const FString& Result)
		{
			UFeatureSaveGameSubsystem* Self = WeakThis.Get();
			if (!Self || Self->OperationId != RequestId) { return; }
			Self->PendingProgress = SavedProgress.Get(FFeatureGameProgress());
			Self->OnLoaded.Broadcast(Self->PendingProgress);
			Self->bSucceeded = true; // Missing/unreadable data completes with storage's zero fallback.
			Self->PublishStatus(Result);
			Self->FinishOperation();
		});
	}
	else
	{
		FFeatureSaveGameStorage::SaveAsync(PendingProgress, [WeakThis, RequestId](bool bSuccess)
		{
			UFeatureSaveGameSubsystem* Self = WeakThis.Get();
			if (!Self || Self->OperationId != RequestId) { return; }
			Self->bSucceeded = bSuccess;
			Self->PublishStatus(bSuccess ? TEXT("Saved successfully") : TEXT("Save failed; quit cancelled"));
			Self->FinishOperation();
		});
	}
}
void UFeatureSaveGameSubsystem::FinishOperation()
{
	bBusy = false;
	UWorld* World = OperationWorld.Get();
	if (World) { UGameplayStatics::SetGamePaused(World, bWasPaused); }
	PublishStatus(bSucceeded ? TEXT("Operation complete; gameplay released") : TEXT("Save failed; gameplay released, game kept open"));
	if (bLoading && bQuitRequested)
	{
		RequestSave(PendingProgress, true);
		return;
	}
	if (bQuitRequested && bSucceeded && World)
	{
		bSavedForQuit = true;
		UKismetSystemLibrary::QuitGame(World, World->GetFirstPlayerController(), EQuitPreference::Quit, false);
	}
}
void UFeatureSaveGameSubsystem::CancelOperation()
{
	++OperationId;
	bBusy = false;
	OperationWorld.Reset();
}
void UFeatureSaveGameSubsystem::PublishStatus(const FString& Status)
{
	StatusHistory.Add(Status);
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureSaveGameSubsystem: %s"), *Status);
	OnStatusChanged.Broadcast(Status, bBusy);
}
const TArray<FString>& UFeatureSaveGameSubsystem::GetStatusHistory() const { return StatusHistory; }
const FString& UFeatureSaveGameSubsystem::GetSlotName() const { return FFeatureSaveGameStorage::GetSlotName(); }
FString UFeatureSaveGameSubsystem::GetSaveFilePath() const { return FFeatureSaveGameStorage::GetSaveFilePath(); }
