#include "FeatureDebugHUDSpawner.h"
#include "Features/UI/FeatureDebugHUD.h"
#include "TestFeatures.h"
#include "GameFramework/PlayerController.h"

UFeatureDebugHUD* FFeatureDebugHUDSpawner::Spawn(APlayerController& PlayerController)
{
	UFeatureDebugHUD* HUD = CreateWidget<UFeatureDebugHUD>(&PlayerController, UFeatureDebugHUD::StaticClass());
	HUD->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	HUD->AddToPlayerScreen(10);
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureDebugHUDSpawner: Overlay added for %s"), *PlayerController.GetName());
	return HUD;
}
