#include "FeatureSaveGameStorage.h"
#include "Features/GameWorkflow/FeatureSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "TestFeatures.h"
#include "Misc/Paths.h"

bool FFeatureSaveGameStorage::HasSave()
{
	return UGameplayStatics::DoesSaveGameExist(GetSlotName(), 0);
}

bool FFeatureSaveGameStorage::DeleteSave()
{
	return !HasSave() || UGameplayStatics::DeleteGameInSlot(GetSlotName(), 0);
}

void FFeatureSaveGameStorage::LoadAsync(TFunction<void(TOptional<FFeatureGameProgress>, const FString&)> Completion)
{
	const bool bExists = HasSave();
	UGameplayStatics::AsyncLoadGameFromSlot(GetSlotName(), 0,
		FAsyncLoadGameFromSlotDelegate::CreateLambda([Completion = MoveTemp(Completion), bExists](const FString&, int32, USaveGame* Data)
		{
			UFeatureSaveGame* Save = Cast<UFeatureSaveGame>(Data);
			const TOptional<FFeatureGameProgress> Progress = Save ? TOptional<FFeatureGameProgress>(Save->Progress) : TOptional<FFeatureGameProgress>();
			Completion(Progress, Save ? FString::Printf(TEXT("Loaded %d total bounces"), Save->Progress.TotalBounces)
				: (bExists ? TEXT("Load failed; starting at zero") : TEXT("No save found; starting at zero")));
		}));
}

void FFeatureSaveGameStorage::SaveAsync(const FFeatureGameProgress& Progress, TFunction<void(bool)> Completion)
{
	UFeatureSaveGame* Save = Cast<UFeatureSaveGame>(UGameplayStatics::CreateSaveGameObject(UFeatureSaveGame::StaticClass()));
	Save->Progress = Progress;
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

	UE_LOG(LogTestFeatures, Log, TEXT("FeatureSaveGameStorage: Loaded %d total bounces"), Save->Progress.TotalBounces);
	Result = FString::Printf(TEXT("Loaded %d total bounces"), Save->Progress.TotalBounces);
	return Save->Progress.TotalBounces;
}

bool FFeatureSaveGameStorage::SaveProgress(const FFeatureGameProgress& Progress)
{
	UFeatureSaveGame* Save = Cast<UFeatureSaveGame>(UGameplayStatics::CreateSaveGameObject(UFeatureSaveGame::StaticClass()));
	Save->Progress = Progress;
	const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, GetSlotName(), 0);
	if (bSaved)
	{
		UE_LOG(LogTestFeatures, Log, TEXT("FeatureSaveGameStorage: Saved %d total bounces"), Progress.TotalBounces);
	}
	else
	{
		UE_LOG(LogTestFeatures, Warning, TEXT("FeatureSaveGameStorage: Failed to save %d total bounces"), Progress.TotalBounces);
	}
	return bSaved;
}

const FString& FFeatureSaveGameStorage::GetSlotName()
{
	static const FString SlotName(TEXT("CrazyBalls"));
	return SlotName;
}

FString FFeatureSaveGameStorage::GetSaveFilePath()
{
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SaveGames"), GetSlotName() + TEXT(".sav")));
}
