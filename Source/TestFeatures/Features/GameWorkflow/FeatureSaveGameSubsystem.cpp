#include "FeatureSaveGameSubsystem.h"
#include "Features_NonUE/FeatureSaveGameStorage.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#if !UE_BUILD_SHIPPING
#include "Misc/App.h"
#endif
#include "TestFeatures.h"

namespace
{
	// Reports whether the debug HUD can drive the persistence previews.
	// Returns bool by value with no pointer or reference parameters.
	bool UsesDebugPreviews()
	{
#if UE_BUILD_SHIPPING
		return false;
#else
		return FApp::CanEverRender();
#endif
	}
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
	bWaitingForIO = true;
	bIOComplete = false;
	bSucceeded = false;
	if (UsesDebugPreviews())
	{
		PublishStatus(bLoad ? TEXT("Loading: preview (game paused)") : TEXT("Saving: preview (game paused)"));
	}
	else
	{
		BeginPendingIO();
	}
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
		FFeatureSaveGameStorage::LoadAsync([WeakThis, RequestId](TOptional<FFeatureGameProgress> SavedProgress, const FString& Result)
		{
			UFeatureSaveGameSubsystem* Self = WeakThis.Get();
			if (!Self || Self->OperationId != RequestId) { return; }
			Self->PendingProgress = SavedProgress.Get(FFeatureGameProgress());
			Self->OnLoaded.Broadcast(Self->PendingProgress);
			Self->bSucceeded = true; // Missing/unreadable data completes with storage's zero fallback.
			Self->bIOComplete = true;
			if (UsesDebugPreviews())
			{
				Self->PublishStatus(Result + TEXT(" (game paused; result preview)"));
			}
			else
			{
				Self->PublishStatus(Result);
				Self->FinishOperation();
			}
		});
	}
	else
	{
		FFeatureSaveGameStorage::SaveAsync(PendingProgress, [WeakThis, RequestId](bool bSuccess)
		{
			UFeatureSaveGameSubsystem* Self = WeakThis.Get();
			if (!Self || Self->OperationId != RequestId) { return; }
			Self->bSucceeded = bSuccess;
			Self->bIOComplete = true;
			if (UsesDebugPreviews())
			{
				Self->PublishStatus(bSuccess ? TEXT("Saved successfully (game paused; result preview)") : TEXT("Save failed; quit cancelled (result preview)"));
			}
			else
			{
				Self->PublishStatus(bSuccess ? TEXT("Saved successfully") : TEXT("Save failed; quit cancelled"));
				Self->FinishOperation();
			}
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
