#include "TrainingSaveGameStorage.h"
#include "Features/TrainingSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "training_UE_584_G2_c.h"
#include "Misc/Paths.h"

int32 FTrainingSaveGameStorage::LoadTotalBounces(FString& Result)
{
	if (!UGameplayStatics::DoesSaveGameExist(GetSlotName(), 0))
	{
		UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingSaveGameStorage: No save yet; starting at zero"));
		Result = TEXT("No save found; starting at zero");
		return 0;
	}

	UTrainingSaveGame* Save = Cast<UTrainingSaveGame>(UGameplayStatics::LoadGameFromSlot(GetSlotName(), 0));
	if (!Save)
	{
		UE_LOG(Logtraining_UE_584_G2_c, Warning, TEXT("TrainingSaveGameStorage: Save could not be read; starting at zero"));
		Result = TEXT("Load failed; starting at zero");
		return 0;
	}

	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingSaveGameStorage: Loaded %d total bounces"), Save->TotalBounces);
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
		UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingSaveGameStorage: Saved %d total bounces"), TotalBounces);
	}
	else
	{
		UE_LOG(Logtraining_UE_584_G2_c, Warning, TEXT("TrainingSaveGameStorage: Failed to save %d total bounces"), TotalBounces);
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
