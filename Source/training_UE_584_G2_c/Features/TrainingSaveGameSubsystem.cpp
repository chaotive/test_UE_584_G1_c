#include "TrainingSaveGameSubsystem.h"
#include "Features_NonUE/TrainingSaveGameStorage.h"

int32 UTrainingSaveGameSubsystem::LoadTotalBounces()
{
	PublishStatus(TEXT("Loading"), true);
	FString Result;
	const int32 TotalBounces = FTrainingSaveGameStorage::LoadTotalBounces(Result);
	PublishStatus(Result, false);
	return TotalBounces;
}

bool UTrainingSaveGameSubsystem::SaveTotalBounces(int32 TotalBounces)
{
	PublishStatus(TEXT("Saving"), true);
	const bool bSaved = FTrainingSaveGameStorage::SaveTotalBounces(TotalBounces);
	PublishStatus(bSaved ? FString::Printf(TEXT("Saved %d total bounces"), TotalBounces) : TEXT("Save failed"), false);
	return bSaved;
}

void UTrainingSaveGameSubsystem::PublishStatus(const FString& Status, bool bBusy)
{
	StatusHistory.Add(Status);
	OnStatusChanged.Broadcast(Status, bBusy);
	if (bBusy)
	{
		OnBeforeIO.Broadcast();
	}
}

const TArray<FString>& UTrainingSaveGameSubsystem::GetStatusHistory() const
{
	return StatusHistory;
}

const FString& UTrainingSaveGameSubsystem::GetSlotName() const
{
	return FTrainingSaveGameStorage::GetSlotName();
}

FString UTrainingSaveGameSubsystem::GetSaveFilePath() const
{
	return FTrainingSaveGameStorage::GetSaveFilePath();
}
