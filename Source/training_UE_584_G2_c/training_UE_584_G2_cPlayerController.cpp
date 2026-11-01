// Copyright Epic Games, Inc. All Rights Reserved.


#include "training_UE_584_G2_cPlayerController.h"
#include "Features/TrainingDebugHUD.h"
#include "Features_NonUE/TrainingDebugHUDSpawner.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "training_UE_584_G2_c.h"
#include "Widgets/Input/SVirtualJoystick.h"

void Atraining_UE_584_G2_cPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		DebugHUD = FTrainingDebugHUDSpawner::Spawn(*this);
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

			UE_LOG(Logtraining_UE_584_G2_c, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void Atraining_UE_584_G2_cPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLocalPlayerController())
	{
		DebugHUD->RemoveFromParent();
	}
	Super::EndPlay(EndPlayReason);
}

void Atraining_UE_584_G2_cPlayerController::SetupInputComponent()
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

bool Atraining_UE_584_G2_cPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
