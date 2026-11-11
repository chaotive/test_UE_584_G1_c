#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FeatureScriptRuntime.h"
#include "TimerManager.h"
#include "FeaturePlayerRules.generated.h"

class APawn;
class UCharacterMovementComponent;
class UFeatureBallManager;

/** Connects player-character script rules to native movement, contacts, and gameplay timers. */
UCLASS()
class TESTFEATURES_API UFeaturePlayerRules : public UActorComponent
{
    GENERATED_BODY()

public:
    // Creates an event-driven component without a per-frame tick.
    // The owning actor retains the default subobject.
    UFeaturePlayerRules();

    // Loads the player rules and applies the supplied movement configuration.
    // Native configuration and transient bonus state remain here.
    void ConfigureSpeed(float BaseMultiplier, float ContactMultiplier, float Duration, bool bStackBonuses);

protected:
    // Observes player-ball contacts once the actor enters gameplay.
    // The event stores a weak UObject binding.
    virtual void BeginPlay() override;

    // Removes the contact binding and expiry timer when gameplay ends.
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    // Allows the focused smoke check to deliver contacts to this component.
    // The test class owns no actor, component, or native runtime references.
    friend class FFeaturePlayerSpeedBoostSpec;

    // Applies the script's next bonus state when this component's pawn is contacted.
    // Pawn is a borrowed world-owned actor reference without ownership transfer.
    void HandleBallContact(APawn& Pawn);

    // Clears expired bonus state and reapplies the scripted baseline speed.
    void ResetSpeed();

    // Writes the script's calculated speed to Unreal's movement component.
    // Native movement remains owned by the actor.
    void ApplySpeed();

    // Retains the script runtime loaded when this character is configured.
    // TUniquePtr exclusively owns the native runtime and releases it with this component.
    TUniquePtr<FFeatureScriptRuntime> Runtime;

    // Identifies the script entry point for contact bonus transitions.
    // Non-owning pointer borrows a function retained by Runtime.
    AngelScript::asIScriptFunction* BonusRule = nullptr;

    // Identifies the script entry point for calculating walking speed.
    // Non-owning pointer borrows a function retained by Runtime.
    AngelScript::asIScriptFunction* SpeedRule = nullptr;

    // Observes the ball manager that publishes character contacts.
    // TWeakObjectPtr does not retain the world-owned subsystem.
    TWeakObjectPtr<UFeatureBallManager> BallManager;

    // Retains the movement component receiving scripted speed results.
    // UPROPERTY tracks the actor-owned UObject reference for garbage collection.
    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> Movement;

    // Tracks the shared gameplay-time expiry for contact bonuses.
    // FTimerHandle identifies a timer without owning the world's timer manager.
    FTimerHandle SpeedTimer;

    // Captures the character's walking speed before this feature is configured.
    float NativeWalkSpeed = 0.0f;

    // Stores the game-mode baseline multiplier supplied to the script.
    float BaseSpeedMultiplier = 0.0f;

    // Stores the game-mode contact multiplier supplied to the script.
    float ContactSpeedMultiplier = 0.0f;

    // Stores the game-mode duration used by the script and native timer.
    float BoostDuration = 0.0f;

    // Stores the configured stacking policy supplied to the script.
    bool bStackSpeedBonuses = false;

    // Tracks the active bonus count returned by the script.
    int32 SpeedBonusCount = 0;
};
