// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Features/GameWorkflow/FeatureGameFlow.h"
#include "GameFramework/GameModeBase.h"
#include "test_UE_584_G1_cGameMode.generated.h"

class UFeatureGameplaySession;
class UFeatureMainMenu;
enum class EFeatureMenuAction : uint8;

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
	// Returns the enum by value without pointer or reference ownership.
	UFUNCTION(BlueprintPure, Category="Game Flow")
	EFeatureGameFlow GetFlowState() const;

	// Pauses an active session and presents the pause menu.
	// No pointer or reference parameters or return value; affects this game mode's world.
	UFUNCTION(BlueprintCallable, Category="Game Flow")
	void PauseSession();

	// Ends an active session with a victory or defeat result.
	// bVictory is passed by value without pointer or reference ownership.
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
	// float stores the non-negative multiplier by value.
	UPROPERTY(EditAnywhere, Category="Feature Player Speed", meta=(ClampMin="0"))
	float FeaturePlayerBaseSpeedMultiplier = 1.0f;

	// Multiplies the base player speed after a ball contact.
	// float stores the non-negative multiplier by value.
	UPROPERTY(EditAnywhere, Category="Feature Player Speed", meta=(ClampMin="0"))
	float FeaturePlayerContactSpeedMultiplier = 2.0f;

	// Enables adding another speed bonus for each contact before the timer expires.
	// bool stores the editor-configured stacking option by value.
	UPROPERTY(EditAnywhere, Category="Feature Player Speed")
	bool bFeaturePlayerContactBoostStacks = false;

	// Sets how long the contact speed boost lasts in gameplay seconds.
	// float stores the non-negative duration by value.
	UPROPERTY(EditAnywhere, Category="Feature Player Speed", meta=(ClampMin="0"))
	float FeaturePlayerContactBoostDuration = 5.0f;

	// Number of training balls to create when play begins.
	// int32 stores the ball count by value; UPROPERTY(EditAnywhere) exposes it for editor configuration without pointer ownership.
	UPROPERTY(EditAnywhere, Category="Training_Ball", meta=(ClampMin="1"))
	int32 FeatureBallCount = 5;

	// Sets the level used when starting a new game.
	// int32 stores the positive starting level by value.
	UPROPERTY(EditAnywhere, Category="Feature Session Limits", meta=(ClampMin="1"))
	int32 FeatureInitialLevel = 1;

	// Sets how many non-player rebounds retire each ball.
	// int32 stores the positive rebound limit by value.
	UPROPERTY(EditAnywhere, Category="Training_Ball", meta=(ClampMin="1"))
	int32 FeatureBallReboundsBeforeRetirement = 5;

	// Sets the uniform size of balls relative to the default sphere.
	// float stores the positive scale by value without object ownership.
	UPROPERTY(EditAnywhere, Category="Training_Ball", meta=(ClampMin="0.01"))
	float FeatureBallSize = 1.0f;

	// Multiplies each ball's initial launch speed.
	// float stores the non-negative multiplier by value without object ownership.
	UPROPERTY(EditAnywhere, Category="Training_Ball", meta=(ClampMin="0"))
	float FeatureBallSpeedMultiplier = 1.0f;

	// Sets how many player-ball blocking contacts win level one.
	// int32 stores a positive threshold by value and exposes it in game-mode defaults.
	UPROPERTY(EditAnywhere, Category="Feature Session Limits", meta=(ClampMin="1"))
	int32 FeaturePlayerContactsToWin = 10;

	// Sets how many ball bounces lose level one.
	// int32 stores a positive threshold by value and exposes it in game-mode defaults.
	UPROPERTY(EditAnywhere, Category="Feature Session Limits", meta=(ClampMin="1"))
	int32 FeatureSessionBouncesToLose = 100;

	// Sets the minimum allowed non-player bounce defeat limit at any level.
	// int32 stores the positive floor by value for editor configuration.
	UPROPERTY(EditAnywhere, Category="Feature Session Limits", meta=(ClampMin="1"))
	int32 FeatureMinimumSessionBouncesToLose = 50;

	// Adds this many required player contacts for each level above level one.
	// int32 stores the non-negative increment by value.
	UPROPERTY(EditAnywhere, Category="Feature Session Limits", meta=(ClampMin="0"))
	int32 FeatureContactsIncreasePerLevel = 1;

	// Subtracts this many allowed non-player bounces for each level above level one.
	// int32 stores the non-negative decrement by value.
	UPROPERTY(EditAnywhere, Category="Feature Session Limits", meta=(ClampMin="0"))
	int32 FeatureBouncesDecreasePerLevel = 1;

	// Minimum corner of the spawn box in world centimeters; each axis must be <= the maximum corner.
	// FVector stores three coordinates by value; UPROPERTY exposes them for editing without a pointer or separate allocation.
	UPROPERTY(EditAnywhere, Category="Training_Ball")
	FVector FeatureBallSpawnMin = FVector(-500.0f, -300.0f, 400.0f);

	// Maximum corner of the spawn box in world centimeters.
	// FVector stores three coordinates by value; UPROPERTY exposes them for editing without a pointer or separate allocation.
	UPROPERTY(EditAnywhere, Category="Training_Ball")
	FVector FeatureBallSpawnMax = FVector(500.0f, 700.0f, 700.0f);

	// Positions the collision enclosure around the arena.
	// FVector is stored by value and exposed for game-mode editing in world centimeters.
	UPROPERTY(EditAnywhere, Category="Feature Bounds")
	FVector FeatureLevelBoundsCenter = FVector(0.0f, 0.0f, 2000.0f);

	// Defines the interior half-size of the session's collision enclosure.
	// FVector is stored by value; editor metadata exposes positive dimensions in centimeters.
	UPROPERTY(EditAnywhere, Category="Feature Bounds", meta=(ClampMin="1"))
	FVector FeatureLevelBoundsHalfExtent = FVector(6000.0f, 6000.0f, 3000.0f);

private:

#if !UE_BUILD_SHIPPING
	friend class FFeatureConsoleCommands;
#endif

	// Tracks which menu or gameplay phase owns the session.
	// Stores the enum by value without pointer or reference ownership.
	EFeatureGameFlow FlowState = EFeatureGameFlow::StartupMenu;

	// Starts a continued session automatically after a defeat retry reloads the level.
	// bool stores the URL option by value without pointer ownership.
	bool bRetrySession = false;

	// Starts the saved session after the reloaded world has begun play.
	// No pointer or reference parameters or return value; the game mode owns the flow transition.
	void StartRetrySession();

	// Retains the active flow screen while it is displayed.
	// UPROPERTY tracks the strong widget pointer for garbage collection.
	UPROPERTY()
	TObjectPtr<UFeatureMainMenu> FlowMenu;

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
	// bVictory is passed by value without pointer or reference ownership.
	void HandleGameplaySessionEnded(bool bVictory);

	// Updates flow and asks the menu class to present or dismiss its screen.
	// NewFlow is passed by value without pointer or reference ownership.
	void SetFlow(EFeatureGameFlow NewFlow);

	// Handles the intent published by the active flow screen.
	// Action is passed by value without pointer or reference ownership.
	void HandleMenuAction(EFeatureMenuAction Action);

	// Retains the gameplay session whose lifecycle events drive player spawning and screens.
	// UPROPERTY tracks a strong TObjectPtr to the world-owned subsystem.
	UPROPERTY()
	TObjectPtr<UFeatureGameplaySession> GameplaySession;
};
