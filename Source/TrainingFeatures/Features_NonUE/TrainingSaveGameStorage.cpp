#include "TrainingSaveGameStorage.h"
#include "Features/TrainingSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "TrainingFeatures.h"
#include "Misc/Paths.h"
#include "Async/Async.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

bool FTrainingSaveGameStorage::ReadSnapshot(const TArray<uint8>& Data, int32& Count)
{
    Count = 0;
    if (Data.IsEmpty()) { return false; }
    // The slot belongs to this storage type; ignore the historical module-qualified class name.
    FMemoryReader Reader = UGameplayStatics::StripSaveGameHeader(Data);
    if (Reader.IsError()) { return false; }
    UTrainingSaveGame* Save = NewObject<UTrainingSaveGame>();
    FObjectAndNameAsStringProxyArchive Archive(Reader, true);
    Save->Serialize(Archive);
    if (Archive.IsError() || Reader.IsError()) { return false; }
    Count = Save->TotalBounces;
    return true;
}

void FTrainingSaveGameStorage::LoadAsync(TFunction<void(int32, const FString&)> Completion)
{
    const bool bExists = UGameplayStatics::DoesSaveGameExist(GetSlotName(), 0);
    Async(EAsyncExecution::ThreadPool, [Completion = MoveTemp(Completion), bExists]() mutable
    {
        TArray<uint8> Data;
        const bool bLoaded = UGameplayStatics::LoadDataFromSlot(Data, GetSlotName(), 0);
        AsyncTask(ENamedThreads::GameThread, [Completion = MoveTemp(Completion), Data = MoveTemp(Data), bLoaded, bExists]()
        {
            int32 Count = 0;
            const bool bReadable = bLoaded && ReadSnapshot(Data, Count);
            Completion(Count, bReadable ? FString::Printf(TEXT("Loaded %d total bounces"), Count)
                : (bExists ? TEXT("Load failed; starting at zero") : TEXT("No save found; starting at zero")));
        });
    });
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

    TArray<uint8> Data;
    int32 Count = 0;
    if (!UGameplayStatics::LoadDataFromSlot(Data, GetSlotName(), 0) || !ReadSnapshot(Data, Count))
    {
        Result = TEXT("Load failed; starting at zero");
        return 0;
    }
    UE_LOG(LogTrainingFeatures, Log, TEXT("TrainingSaveGameStorage: Loaded %d total bounces"), Count);
    Result = FString::Printf(TEXT("Loaded %d total bounces"), Count);
    return Count;
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
