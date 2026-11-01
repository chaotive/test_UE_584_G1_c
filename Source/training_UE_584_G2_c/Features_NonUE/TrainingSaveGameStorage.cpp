#include "TrainingSaveGameStorage.h"
#include "Features/TrainingSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "training_UE_584_G2_c.h"

int32 FTrainingSaveGameStorage::LoadTotalBounces()
{
	if (!UGameplayStatics::DoesSaveGameExist(TEXT("TrainingSandbox"), 0))
	{
		UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingSaveGameStorage: No save yet; starting at zero"));
		return 0;
	}

	UTrainingSaveGame* Save = Cast<UTrainingSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("TrainingSandbox"), 0));
	if (!Save)
	{
		UE_LOG(Logtraining_UE_584_G2_c, Warning, TEXT("TrainingSaveGameStorage: Save could not be read; starting at zero"));
		return 0;
	}

	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingSaveGameStorage: Loaded %d total bounces"), Save->TotalBounces);
	return Save->TotalBounces;
}

bool FTrainingSaveGameStorage::SaveTotalBounces(int32 TotalBounces)
{
	UTrainingSaveGame* Save = Cast<UTrainingSaveGame>(UGameplayStatics::CreateSaveGameObject(UTrainingSaveGame::StaticClass()));
	Save->TotalBounces = TotalBounces;
	const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, TEXT("TrainingSandbox"), 0);
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
