// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Features/GameWorkflow/FeatureGameFlow.h"
#include "GameFramework/GameModeBase.h"
#include "test_UE_584_G1_cGameMode.generated.h"

class UFeatureGameplaySession;
class UFeatureMenuController;

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class Atest_UE_584_G1_cGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	// No pointer or reference parameters; Unreal creates and owns this game mode instance.
	Atest_UE_584_G1_cGameMode();

	// MapName and Options are read-only references; ErrorMessage is a mutable reference for reporting initialization errors.
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	// No pointer or reference parameters; initializes the game state owned by the world.
	virtual void InitGameState() override;
	// No pointer or reference parameters; starts play for this game mode's world.
	virtual void StartPlay() override;

	// Reports the current flow for other gameplay systems.
	UFUNCTION(BlueprintPure, Category="Game Flow")
	EFeatureGameFlow GetFlowState() const;

	// Pauses an active session and presents the pause menu.
	// No pointer or reference parameters or return value; affects this game mode's world.
	UFUNCTION(BlueprintCallable, Category="Game Flow")
	void PauseSession();

	// Ends an active session with a victory or defeat result.
	UFUNCTION(BlueprintCallable, Category="Game Flow")
	void FinishSession(bool bVictory);

protected:

	// No pointer or reference parameters; completes this actor's component initialization.
	virtual void PostInitializeComponents() override;
	// Logs the world lifecycle while the main menu is open.
	// No pointer or reference parameters; Unreal owns the game mode.
	virtual void BeginPlay() override;
	// EndPlayReason is passed by value; no pointer or reference ownership is transferred.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Defers the default pawn spawn until the startup menu starts gameplay.
	// NewPlayer is a borrowed engine-owned controller pointer.
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	// Multiplies the character's configured walking speed throughout a session.
	// EditAnywhere exposes the baseline for arena tuning. ClampMin allows zero and excludes negative editor values.
	UPROPERTY(EditAnywhere, Category="Feature Player Speed", meta=(ClampMin="0"))
	float FeaturePlayerBaseSpeedMultiplier = 1.0f;

	// Multiplies the base player speed after a ball contact.
	// EditAnywhere exposes the contact boost for tuning. ClampMin excludes negative editor values.
	UPROPERTY(EditAnywhere, Category="Feature Player Speed", meta=(ClampMin="0"))
	float FeaturePlayerContactSpeedMultiplier = 2.0f;

	// Enables adding another speed bonus for each contact before the timer expires.
	// EditAnywhere lets the game-mode defaults choose between refreshing a boost and stacking contacts.
	UPROPERTY(EditAnywhere, Category="Feature Player Speed")
	bool bFeaturePlayerContactBoostStacks = false;

	// Sets how long the contact speed boost lasts in gameplay seconds.
	// EditAnywhere exposes the boost duration. ClampMin allows zero to disable the timed boost.
	UPROPERTY(EditAnywhere, Category="Feature Player Speed", meta=(ClampMin="0"))
	float FeaturePlayerContactBoostDuration = 5.0f;

	// Number of training balls to create when play begins.
	// EditAnywhere lets an arena set the initial volley size. ClampMin keeps editor values at one ball or more.
	UPROPERTY(EditAnywhere, Category="Training_Ball", meta=(ClampMin="1"))
	int32 FeatureBallCount = 5;

	// Sets the level used when starting a new game.
	// EditAnywhere allows a different starting level in game-mode defaults. ClampMin keeps it at one or higher.
	UPROPERTY(EditAnywhere, Category="Feature Session Limits", meta=(ClampMin="1"))
	int32 FeatureInitialLevel = 1;

	// Sets how many non-player rebounds retire each ball.
	// EditAnywhere exposes the ball lifetime for tuning. ClampMin requires at least one rebound in the editor.
	UPROPERTY(EditAnywhere, Category="Training_Ball", meta=(ClampMin="1"))
	int32 FeatureBallReboundsBeforeRetirement = 5;

	// Sets the uniform size of balls relative to the default sphere.
	// EditAnywhere allows arena-specific ball sizes. ClampMin keeps the scale above zero in the editor.
	UPROPERTY(EditAnywhere, Category="Training_Ball", meta=(ClampMin="0.01"))
	float FeatureBallSize = 1.0f;

	// Multiplies each ball's initial launch speed.
	// EditAnywhere exposes launch speed for tuning. ClampMin allows stationary launches and excludes negative editor values.
	UPROPERTY(EditAnywhere, Category="Training_Ball", meta=(ClampMin="0"))
	float FeatureBallSpeedMultiplier = 1.0f;

	// Sets how many player-ball blocking contacts win level one.
	// EditAnywhere exposes the starting win target. ClampMin requires at least one contact in the editor.
	UPROPERTY(EditAnywhere, Category="Feature Session Limits", meta=(ClampMin="1"))
	int32 FeaturePlayerContactsToWin = 10;

	// Sets how many ball bounces lose level one.
	// EditAnywhere exposes the starting loss target. ClampMin keeps it positive in the editor.
	UPROPERTY(EditAnywhere, Category="Feature Session Limits", meta=(ClampMin="1"))
	int32 FeatureSessionBouncesToLose = 100;

	// Sets the minimum allowed non-player bounce defeat limit at any level.
	// EditAnywhere exposes the loss floor for tuning. ClampMin keeps that floor positive in the editor.
	UPROPERTY(EditAnywhere, Category="Feature Session Limits", meta=(ClampMin="1"))
	int32 FeatureMinimumSessionBouncesToLose = 50;

	// Adds this many required player contacts for each level above level one.
	// EditAnywhere lets game-mode defaults tune difficulty growth. ClampMin allows a flat win target.
	UPROPERTY(EditAnywhere, Category="Feature Session Limits", meta=(ClampMin="0"))
	int32 FeatureContactsIncreasePerLevel = 1;

	// Subtracts this many allowed non-player bounces for each level above level one.
	// EditAnywhere lets game-mode defaults tune difficulty growth. ClampMin allows a flat loss target.
	UPROPERTY(EditAnywhere, Category="Feature Session Limits", meta=(ClampMin="0"))
	int32 FeatureBouncesDecreasePerLevel = 1;

	// Minimum corner of the spawn box in world centimeters; each axis must be <= the maximum corner.
	// EditAnywhere lets each arena tune the lower corner of the spawn region.
	UPROPERTY(EditAnywhere, Category="Training_Ball")
	FVector FeatureBallSpawnMin = FVector(-500.0f, -300.0f, 400.0f);

	// Maximum corner of the spawn box in world centimeters.
	// EditAnywhere lets each arena tune the upper corner of the spawn region.
	UPROPERTY(EditAnywhere, Category="Training_Ball")
	FVector FeatureBallSpawnMax = FVector(500.0f, 700.0f, 700.0f);

	// Positions the collision enclosure around the arena.
	// EditAnywhere lets the arena place the enclosure independently of the ball spawn region.
	UPROPERTY(EditAnywhere, Category="Feature Bounds")
	FVector FeatureLevelBoundsCenter = FVector(0.0f, 0.0f, 2000.0f);

	// Defines the interior half-size of the session's collision enclosure.
	// EditAnywhere exposes the enclosure size. ClampMin keeps each editor dimension above zero.
	UPROPERTY(EditAnywhere, Category="Feature Bounds", meta=(ClampMin="1"))
	FVector FeatureLevelBoundsHalfExtent = FVector(6000.0f, 6000.0f, 3000.0f);

private:

	// Tracks which menu or gameplay phase owns the session.
	EFeatureGameFlow FlowState = EFeatureGameFlow::StartupMenu;

	// Starts a continued session automatically after a defeat retry reloads the level.
	bool bRetrySession = false;

	// Starts the saved session after the reloaded world has begun play.
	// No pointer or reference parameters or return value; the game mode owns the flow transition.
	void StartRetrySession();

	// Retains the world subsystem that presents menus and executes their actions.
	// UPROPERTY tracks a strong reference to the world-owned controller.
	UPROPERTY()
	TObjectPtr<UFeatureMenuController> MenuController;

	// Creates the player and presents gameplay when the session starts.
	// No pointer or reference parameters or return value; responds to a session event.
	void HandleGameplaySessionStarted();

	// Presents gameplay when the session resumes.
	// No pointer or reference parameters or return value; responds to a session event.
	void HandleGameplaySessionResumed();

	// Presents the pause workflow after a gameplay pause request succeeds.
	// No pointer or reference parameters or return value; updates this game mode's flow.
	void HandleGameplaySessionPaused();

	// Presents the result workflow after a gameplay objective ends the session.
	void HandleGameplaySessionEnded(bool bVictory);

	// Updates flow and asks the menu class to present or dismiss its screen.
	void SetFlow(EFeatureGameFlow NewFlow);

	// Starts gameplay using the configured arena and ball settings.
	void StartGameplaySession(bool bContinue);

	// Retains the gameplay session whose lifecycle events drive player spawning and screens.
	// UPROPERTY tracks a strong TObjectPtr to the world-owned subsystem.
	UPROPERTY()
	TObjectPtr<UFeatureGameplaySession> GameplaySession;
};
