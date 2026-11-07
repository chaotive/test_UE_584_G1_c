// Copyright Epic Games, Inc. All Rights Reserved.


#include "test_UE_584_G1_cPlayerController.h"
#include "test_UE_584_G1_cGameMode.h"
#include "Features/FeatureDebugHUD.h"
#if !UE_BUILD_SHIPPING
#include "Features_NonUE/FeatureDebugHUDSpawner.h"
#endif
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "test_UE_584_G1_c.h"
#include "Widgets/Input/SVirtualJoystick.h"

void Atest_UE_584_G1_cPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IsLocalPlayerController())
	{
		CreateGameplayWidgets();
	}
}

void Atest_UE_584_G1_cPlayerController::RequestPause()
{
	GetWorld()->GetAuthGameMode<Atest_UE_584_G1_cGameMode>()->PauseSession();
}

void Atest_UE_584_G1_cPlayerController::CreateGameplayWidgets()
{
#if !UE_BUILD_SHIPPING
	if (!DebugHUD)
	{
		DebugHUD = FFeatureDebugHUDSpawner::Spawn(*this);
	}
#endif
	if (!MobileControlsWidget && ShouldUseTouchControls())
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
	if (DebugHUD)
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
		InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &Atest_UE_584_G1_cPlayerController::RequestPause);
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
