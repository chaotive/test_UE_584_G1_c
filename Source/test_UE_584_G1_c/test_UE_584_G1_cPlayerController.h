// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "test_UE_584_G1_cPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UFeatureDebugHUD;
class UFeatureGameplayHUD;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings and creates local player UI
 */
UCLASS(abstract)
class Atest_UE_584_G1_cPlayerController : public APlayerController
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

	// Creates the local gameplay widgets on the first pawn possession.
	// InPawn is a borrowed engine-owned pawn pointer.
	virtual void OnPossess(APawn* InPawn) override;

	// Removes the gameplay overlay when this controller stops playing.
	// EndPlayReason is passed by value; no pointer or reference ownership is transferred.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Keeps the runtime debug overlay available throughout this player's session.
	// UPROPERTY tracks a strong TObjectPtr so garbage collection retains the player-owned widget.
	UPROPERTY()
	TObjectPtr<UFeatureDebugHUD> DebugHUD;

	// Retains the gameplay objective display for this local player.
	// UPROPERTY tracks the strong widget pointer for garbage collection.
	UPROPERTY()
	TObjectPtr<UFeatureGameplayHUD> GameplayHUD;

	// Registers the local player's input mapping contexts and pause request.
	// No pointer or reference parameters or return value; configures this controller's input subsystem.
	virtual void SetupInputComponent() override;

	// Reports whether platform settings or the forced setting request touch controls.
	// Returns bool by value without pointers or references; const keeps this query read-only.
	bool ShouldUseTouchControls() const;

private:
	// Forwards the gameplay pause input to the game mode.
	// No pointer or reference parameters or return value.
	void RequestPause();

	// Creates the gameplay overlay and optional mobile controls.
	// No pointer or reference parameters or return value; this controller retains the widgets.
	void CreateGameplayWidgets();
};
