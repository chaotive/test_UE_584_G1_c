#include "FeatureSaveGameSubsystem.h"
#include "Features_NonUE/FeatureSaveGameStorage.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/App.h"
#include "TestFeatures.h"

void UFeatureSaveGameSubsystem::RequestLoad()
{
	StartOperation(true, 0, false);
}
void UFeatureSaveGameSubsystem::RequestSave(int32 Count, bool bQuitAfter)
{
	StartOperation(false, Count, bQuitAfter);
}
void UFeatureSaveGameSubsystem::StartOperation(bool bLoad, int32 Count, bool bQuitAfter)
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
	PendingCount = Count;
	bLoading = bLoad;
	bQuitRequested = bQuitAfter;
	bBusy = true;
	bWaitingForIO = true;
	bIOComplete = false;
	bSucceeded = false;
	PublishStatus(bLoad ? TEXT("Loading: preview (game paused)") : TEXT("Saving: preview (game paused)"));
	if (!FApp::CanEverRender()) { BeginPendingIO(); }
}
void UFeatureSaveGameSubsystem::BeginPendingIO()
{
	if (!bBusy || !bWaitingForIO) { return; }
	bWaitingForIO = false;
	PublishStatus(bLoading ? TEXT("Loading: disk I/O (game paused)") : TEXT("Saving: disk I/O (game paused)"));
	const uint32 RequestId = OperationId;
	TWeakObjectPtr<UFeatureSaveGameSubsystem> WeakThis(this);
	if (bLoading)
	{
		FFeatureSaveGameStorage::LoadAsync([WeakThis, RequestId](TOptional<int32> SavedCount, const FString& Result)
		{
			UFeatureSaveGameSubsystem* Self = WeakThis.Get();
			if (!Self || Self->OperationId != RequestId) { return; }
			const int32 Count = SavedCount.Get(0);
			Self->PendingCount = Count;
			Self->OnLoaded.Broadcast(Count);
			Self->bSucceeded = true; // Missing/unreadable data completes with storage's zero fallback.
			Self->bIOComplete = true;
			Self->PublishStatus(Result + TEXT(" (game paused; result preview)"));
			if (!FApp::CanEverRender()) { Self->FinishOperation(); }
		});
	}
	else
	{
		FFeatureSaveGameStorage::SaveAsync(PendingCount, [WeakThis, RequestId](bool bSuccess)
		{
			UFeatureSaveGameSubsystem* Self = WeakThis.Get();
			if (!Self || Self->OperationId != RequestId) { return; }
			Self->bSucceeded = bSuccess;
			Self->bIOComplete = true;
			Self->PublishStatus(bSuccess ? TEXT("Saved successfully (game paused; result preview)") : TEXT("Save failed; quit cancelled (result preview)"));
			if (!FApp::CanEverRender()) { Self->FinishOperation(); }
		});
	}
}
void UFeatureSaveGameSubsystem::FinishOperation()
{
	if (!bBusy || !bIOComplete) { return; }
	bBusy = false;
	bIOComplete = false;
	UWorld* World = OperationWorld.Get();
	if (World) { UGameplayStatics::SetGamePaused(World, bWasPaused); }
	PublishStatus(bSucceeded ? TEXT("Operation complete; gameplay released") : TEXT("Save failed; gameplay released, game kept open"));
	if (bLoading && bQuitRequested)
	{
		RequestSave(PendingCount, true);
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
	bBusy = bWaitingForIO = bIOComplete = false;
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
