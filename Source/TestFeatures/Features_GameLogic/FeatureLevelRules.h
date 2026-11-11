#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "FeatureScriptRuntime.h"
#include "Subsystems/WorldSubsystem.h"
#include "FeatureLevelRules.generated.h"

enum class EFeatureGameplaySessionResult : uint8;

/** Hosts value-only gameplay rules without exposing world objects to scripts. */
UCLASS()
class TESTFEATURES_API UFeatureLevelRules : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Loads and compiles the level rules once for this world.
    // Collection is a borrowed subsystem-collection reference; this subsystem owns the native runtime.
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Releases the execution context before shutting down its script engine.
    // Releases subsystem-owned native resources.
    virtual void Deinitialize() override;

    // Invokes the script's player-contact threshold rule.
    int32 PlayerContactsToWin(int32 Level, int32 BaseContacts, int32 IncreasePerLevel) const;

    // Invokes the script's non-player bounce threshold rule.
    int32 SessionBouncesToLose(int32 Level, int32 BaseBounces, int32 DecreasePerLevel, int32 MinimumBounces) const;

    // Chooses the current session result, including simultaneous-limit precedence.
    EFeatureGameplaySessionResult SessionOutcome(int32 PlayerContacts, int32 Bounces, int32 WinLimit, int32 LossLimit) const;

    // Calculates the saved level after recording a session result.
    int32 NextLevel(int32 Level, EFeatureGameplaySessionResult Result) const;

private:
    // Makes the native result enum available to the script without mirrored numeric definitions.
    // Engine is a borrowed SDK reference; enum values remain defined by EFeatureGameplaySessionResult.
    static void RegisterTypes(AngelScript::asIScriptEngine& Engine);

    // Passes integer arguments to a compiled script function and reads its integer result.
    // Function is borrowed from the engine; TConstArrayView borrows a contiguous input array without copying it.
    int32 Execute(AngelScript::asIScriptFunction& Function, TConstArrayView<int32> Arguments) const;

    // Owns the compiler, module and functions for this world's rules.
    // TUniquePtr exclusively owns the native runtime until subsystem deinitialization.
    TUniquePtr<FFeatureScriptRuntime> Runtime;

    // Identifies the script function for the victory threshold.
    // Non-owning native pointer borrows a function retained by the script module.
    AngelScript::asIScriptFunction* WinRule = nullptr;

    // Identifies the script function for the defeat threshold.
    // Non-owning native pointer borrows a function retained by the script module.
    AngelScript::asIScriptFunction* LossRule = nullptr;

    // Identifies the script entry point that chooses a session outcome.
    // Non-owning native pointer borrows a function retained by Runtime.
    AngelScript::asIScriptFunction* OutcomeRule = nullptr;

    // Identifies the script entry point that advances or retains the saved level.
    // Non-owning native pointer borrows a function retained by Runtime.
    AngelScript::asIScriptFunction* ProgressionRule = nullptr;
};
