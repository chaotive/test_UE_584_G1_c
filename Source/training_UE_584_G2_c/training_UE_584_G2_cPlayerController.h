// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "training_UE_584_G2_cPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UTrainingDebugHUD;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings and creates local player UI
 */
UCLASS(abstract)
class Atraining_UE_584_G2_cPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	// Lists the input mapping assets enabled for every local player.
	// UPROPERTY tracks the asset references in this growable TArray of raw UObject pointers; callers do not delete the assets.
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	// Lists additional input mappings enabled when touch controls are not in use.
	// UPROPERTY tracks the asset references in this growable TArray of raw UObject pointers; callers do not delete the assets.
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	// Selects the widget class used for mobile touch controls.
	// TSubclassOf restricts this reflected class reference to UUserWidget subclasses rather than storing a widget instance.
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	// Retains the mobile controls widget created for this player.
	// UPROPERTY tracks a strong TObjectPtr so garbage collection retains the player-owned widget.
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	// Enables touch controls even when running on a non-mobile platform.
	// bool is stored by value; UPROPERTY exposes it for editing and Config allows configuration-file defaults.
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	// Creates the local debug overlay and optional mobile controls when play begins.
	// No pointer or reference parameters or return value; initializes widgets retained by this controller.
	virtual void BeginPlay() override;

	// Removes the local debug overlay when this player controller stops playing.
	// EndPlayReason is passed by value; no pointer or reference ownership is transferred.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Keeps the runtime debug overlay available throughout this player's session.
	// UPROPERTY tracks a strong TObjectPtr so garbage collection retains the player-owned widget.
	UPROPERTY()
	TObjectPtr<UTrainingDebugHUD> DebugHUD;

	// Registers the local player's input mapping contexts.
	// No pointer or reference parameters or return value; configures this controller's input subsystem.
	virtual void SetupInputComponent() override;

	// Reports whether platform settings or the forced setting request touch controls.
	// Returns bool by value without pointers or references; const keeps this query read-only.
	bool ShouldUseTouchControls() const;

};
