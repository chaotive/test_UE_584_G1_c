#include "FeatureSaveGameStorage.h"
#include "Features/FeatureSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "TestFeatures.h"
#include "Misc/Paths.h"

bool FFeatureSaveGameStorage::HasSave()
{
	return UGameplayStatics::DoesSaveGameExist(GetSlotName(), 0);
}

void FFeatureSaveGameStorage::LoadAsync(TFunction<void(TOptional<int32>, const FString&)> Completion)
{
	const bool bExists = HasSave();
	UGameplayStatics::AsyncLoadGameFromSlot(GetSlotName(), 0,
		FAsyncLoadGameFromSlotDelegate::CreateLambda([Completion = MoveTemp(Completion), bExists](const FString&, int32, USaveGame* Data)
		{
			UFeatureSaveGame* Save = Cast<UFeatureSaveGame>(Data);
			const TOptional<int32> Count = Save ? TOptional<int32>(Save->TotalBounces) : TOptional<int32>();
			Completion(Count, Save ? FString::Printf(TEXT("Loaded %d total bounces"), Save->TotalBounces)
				: (bExists ? TEXT("Load failed; starting at zero") : TEXT("No save found; starting at zero")));
		}));
}

void FFeatureSaveGameStorage::SaveAsync(int32 Count, TFunction<void(bool)> Completion)
{
	UFeatureSaveGame* Save = Cast<UFeatureSaveGame>(UGameplayStatics::CreateSaveGameObject(UFeatureSaveGame::StaticClass()));
	Save->TotalBounces = Count;
	UGameplayStatics::AsyncSaveGameToSlot(Save, GetSlotName(), 0,
		FAsyncSaveGameToSlotDelegate::CreateLambda([Completion = MoveTemp(Completion)](const FString&, int32, bool bSuccess)
		{
			Completion(bSuccess);
		}));
}

int32 FFeatureSaveGameStorage::LoadTotalBounces(FString& Result)
{
	if (!HasSave())
	{
		UE_LOG(LogTestFeatures, Log, TEXT("FeatureSaveGameStorage: No save yet; starting at zero"));
		Result = TEXT("No save found; starting at zero");
		return 0;
	}

	UFeatureSaveGame* Save = Cast<UFeatureSaveGame>(UGameplayStatics::LoadGameFromSlot(GetSlotName(), 0));
	if (!Save)
	{
		UE_LOG(LogTestFeatures, Warning, TEXT("FeatureSaveGameStorage: Save could not be read; starting at zero"));
		Result = TEXT("Load failed; starting at zero");
		return 0;
	}

	UE_LOG(LogTestFeatures, Log, TEXT("FeatureSaveGameStorage: Loaded %d total bounces"), Save->TotalBounces);
	Result = FString::Printf(TEXT("Loaded %d total bounces"), Save->TotalBounces);
	return Save->TotalBounces;
}

bool FFeatureSaveGameStorage::SaveTotalBounces(int32 TotalBounces)
{
	UFeatureSaveGame* Save = Cast<UFeatureSaveGame>(UGameplayStatics::CreateSaveGameObject(UFeatureSaveGame::StaticClass()));
	Save->TotalBounces = TotalBounces;
	const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, GetSlotName(), 0);
	if (bSaved)
	{
		UE_LOG(LogTestFeatures, Log, TEXT("FeatureSaveGameStorage: Saved %d total bounces"), TotalBounces);
	}
	else
	{
		UE_LOG(LogTestFeatures, Warning, TEXT("FeatureSaveGameStorage: Failed to save %d total bounces"), TotalBounces);
	}
	return bSaved;
}

const FString& FFeatureSaveGameStorage::GetSlotName()
{
	static const FString SlotName(TEXT("TrainingSandbox"));
	return SlotName;
}

FString FFeatureSaveGameStorage::GetSaveFilePath()
{
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SaveGames"), GetSlotName() + TEXT(".sav")));
}
