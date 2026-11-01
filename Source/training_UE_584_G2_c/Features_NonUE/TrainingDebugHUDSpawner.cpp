#include "TrainingDebugHUDSpawner.h"
#include "Features/TrainingDebugHUD.h"
#include "training_UE_584_G2_c.h"
#include "GameFramework/PlayerController.h"

UTrainingDebugHUD* FTrainingDebugHUDSpawner::Spawn(APlayerController& PlayerController)
{
	UTrainingDebugHUD* HUD = CreateWidget<UTrainingDebugHUD>(&PlayerController, UTrainingDebugHUD::StaticClass());
	HUD->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	HUD->AddToPlayerScreen(10);
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingDebugHUDSpawner: Overlay added for %s"), *PlayerController.GetName());
	return HUD;
}
