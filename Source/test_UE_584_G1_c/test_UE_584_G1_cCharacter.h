// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/TimerHandle.h"
#include "Logging/LogMacros.h"
#include "test_UE_584_G1_cCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class UFeatureGameplaySession;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class Atest_UE_584_G1_cCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

public:

	/** Constructor */
	Atest_UE_584_G1_cCharacter();

	// Applies the configured base speed and contact boost settings to this player.
	// Multipliers, duration, and stacking flag are values without pointer or reference ownership.
	void ConfigureFeatureSpeed(float BaseMultiplier, float ContactMultiplier, float Duration, bool bStackBonuses);

protected:

	// Subscribes this player to ball contact events when gameplay begins.
	// No pointer or reference parameters or return value; the world owns the session.
	virtual void BeginPlay() override;

	// Removes the contact subscription when the player leaves the world.
	// EndPlayReason is passed by value without ownership transfer.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

private:

	// Applies or refreshes the temporary boost when this player contacts a ball.
	// Pawn is a borrowed world-owned reference without ownership transfer.
	void HandleFeatureBallContact(APawn& Pawn);

	// Restores the configured base walking speed after the boost expires.
	// No pointer or reference parameters or return value.
	void ResetFeatureSpeed();

	// Observes the world-owned gameplay session for contact events.
	// TWeakObjectPtr does not retain the subsystem.
	TWeakObjectPtr<UFeatureGameplaySession> FeatureSession;

	// Tracks the gameplay-time timer that restores normal speed.
	// FTimerHandle stores a value handle without owning the timer manager.
	FTimerHandle FeatureSpeedTimer;

	// Stores the configured baseline walking speed in centimeters per second.
	// float stores the speed by value.
	float FeatureBaseWalkSpeed = 0.0f;

	// Stores the multiplier applied during the contact boost.
	// float stores the multiplier by value.
	float FeatureContactMultiplier = 2.0f;

	// Enables accumulating contact bonuses until the shared timer expires.
	// bool stores the configured stacking option by value.
	bool bFeatureStackSpeedBonuses = false;

	// Counts active bonuses that share the most recent contact timer.
	// int32 stores the count by value without ownership.
	int32 FeatureSpeedBonusCount = 0;

	// Stores the boost duration in gameplay seconds.
	// float stores the duration by value.
	float FeatureBoostDuration = 5.0f;

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

