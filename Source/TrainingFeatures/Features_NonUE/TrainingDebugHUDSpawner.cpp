#include "TrainingDebugHUDSpawner.h"
#include "Features/TrainingDebugHUD.h"
#include "TrainingFeatures.h"
#include "GameFramework/PlayerController.h"

UTrainingDebugHUD* FTrainingDebugHUDSpawner::Spawn(APlayerController& PlayerController)
{
	UTrainingDebugHUD* HUD = CreateWidget<UTrainingDebugHUD>(&PlayerController, UTrainingDebugHUD::StaticClass());
	HUD->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	HUD->AddToPlayerScreen(10);
	UE_LOG(LogTrainingFeatures, Log, TEXT("TrainingDebugHUDSpawner: Overlay added for %s"), *PlayerController.GetName());
	return HUD;
}
