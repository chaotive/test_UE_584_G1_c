#include "TrainingSaveGameStorage.h"
#include "Features/TrainingSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "TrainingFeatures.h"
#include "Misc/Paths.h"

void FTrainingSaveGameStorage::LoadAsync(TFunction<void(int32, const FString&)> Completion)
{
	const bool bExists = UGameplayStatics::DoesSaveGameExist(GetSlotName(), 0);
	UGameplayStatics::AsyncLoadGameFromSlot(GetSlotName(), 0,
		FAsyncLoadGameFromSlotDelegate::CreateLambda([Completion = MoveTemp(Completion), bExists](const FString&, int32, USaveGame* Data)
		{
			UTrainingSaveGame* Save = Cast<UTrainingSaveGame>(Data);
			const int32 Count = Save ? Save->TotalBounces : 0;
			Completion(Count, Save ? FString::Printf(TEXT("Loaded %d total bounces"), Count)
				: (bExists ? TEXT("Load failed; starting at zero") : TEXT("No save found; starting at zero")));
		}));
}

void FTrainingSaveGameStorage::SaveAsync(int32 Count, TFunction<void(bool)> Completion)
{
	UTrainingSaveGame* Save = Cast<UTrainingSaveGame>(UGameplayStatics::CreateSaveGameObject(UTrainingSaveGame::StaticClass()));
	Save->TotalBounces = Count;
	UGameplayStatics::AsyncSaveGameToSlot(Save, GetSlotName(), 0,
		FAsyncSaveGameToSlotDelegate::CreateLambda([Completion = MoveTemp(Completion)](const FString&, int32, bool bSuccess)
		{
			Completion(bSuccess);
		}));
}

int32 FTrainingSaveGameStorage::LoadTotalBounces(FString& Result)
{
	if (!UGameplayStatics::DoesSaveGameExist(GetSlotName(), 0))
	{
		UE_LOG(LogTrainingFeatures, Log, TEXT("TrainingSaveGameStorage: No save yet; starting at zero"));
		Result = TEXT("No save found; starting at zero");
		return 0;
	}

	UTrainingSaveGame* Save = Cast<UTrainingSaveGame>(UGameplayStatics::LoadGameFromSlot(GetSlotName(), 0));
	if (!Save)
	{
		UE_LOG(LogTrainingFeatures, Warning, TEXT("TrainingSaveGameStorage: Save could not be read; starting at zero"));
		Result = TEXT("Load failed; starting at zero");
		return 0;
	}

	UE_LOG(LogTrainingFeatures, Log, TEXT("TrainingSaveGameStorage: Loaded %d total bounces"), Save->TotalBounces);
	Result = FString::Printf(TEXT("Loaded %d total bounces"), Save->TotalBounces);
	return Save->TotalBounces;
}

bool FTrainingSaveGameStorage::SaveTotalBounces(int32 TotalBounces)
{
	UTrainingSaveGame* Save = Cast<UTrainingSaveGame>(UGameplayStatics::CreateSaveGameObject(UTrainingSaveGame::StaticClass()));
	Save->TotalBounces = TotalBounces;
	const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, GetSlotName(), 0);
	if (bSaved)
	{
		UE_LOG(LogTrainingFeatures, Log, TEXT("TrainingSaveGameStorage: Saved %d total bounces"), TotalBounces);
	}
	else
	{
		UE_LOG(LogTrainingFeatures, Warning, TEXT("TrainingSaveGameStorage: Failed to save %d total bounces"), TotalBounces);
	}
	return bSaved;
}

const FString& FTrainingSaveGameStorage::GetSlotName()
{
	static const FString SlotName(TEXT("TrainingSandbox"));
	return SlotName;
}

FString FTrainingSaveGameStorage::GetSaveFilePath()
{
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SaveGames"), GetSlotName() + TEXT(".sav")));
}
