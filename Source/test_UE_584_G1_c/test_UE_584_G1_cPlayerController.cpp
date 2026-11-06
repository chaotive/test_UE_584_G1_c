// Copyright Epic Games, Inc. All Rights Reserved.


#include "test_UE_584_G1_cPlayerController.h"
#include "Features/FeatureDebugHUD.h"
#include "Features_NonUE/FeatureDebugHUDSpawner.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "test_UE_584_G1_c.h"
#include "Widgets/Input/SVirtualJoystick.h"

void Atest_UE_584_G1_cPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		DebugHUD = FFeatureDebugHUDSpawner::Spawn(*this);
	}

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(Logtest_UE_584_G1_c, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void Atest_UE_584_G1_cPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLocalPlayerController())
	{
		DebugHUD->RemoveFromParent();
	}
	Super::EndPlay(EndPlayReason);
}

void Atest_UE_584_G1_cPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool Atest_UE_584_G1_cPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
