#pragma once

#include "CoreMinimal.h"
#include "FeatureScriptRuntime.h"
#include "Subsystems/WorldSubsystem.h"
#include "FeatureBallRules.generated.h"

class AActor;

// Defines the native effects a scripted ball-contact decision may request.
// uint8 flags are copied values; their numeric definitions are registered with AngelScript from this enum.
enum class EFeatureBallContactAction : uint8
{
    None = 0,
    CountBounce = 1,
    Retire = 2
};
ENUM_CLASS_FLAGS(EFeatureBallContactAction);

/** Translates native collision facts into scripted ball-lifetime decisions. */
UCLASS()
class TESTFEATURES_API UFeatureBallRules : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Compiles the ball rules once for this world.
    // Collection is a borrowed subsystem reference; this subsystem exclusively owns its runtime.
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Releases the compiled rules when the world is torn down.
    // No pointer or reference arguments or return value; releases the owned native runtime.
    virtual void Deinitialize() override;

    // Translates collision facts and asks the script which ball effects to apply.
    // OtherActor is a borrowed nullable world-owned pointer; the remaining inputs and flags are copied values.
    EFeatureBallContactAction EvaluateContact(AActor* OtherActor, bool bHasSolverImpulse, int32 BounceCount, int32 ReboundLimit) const;

    // Asks the script whether the current session phase permits replacement.
    // The bool input and result are copied values without pointers or references.
    bool ShouldReplaceBall(bool bSessionPlaying) const;

private:
    // Registers the native action flags before the script is compiled.
    // Engine is a borrowed SDK reference; the enum remains the only source of numeric action values.
    static void RegisterTypes(AngelScript::asIScriptEngine& Engine);

    // Retains the ball script's compiler, module, and execution context.
    // TUniquePtr exclusively owns the runtime until subsystem deinitialization.
    TUniquePtr<FFeatureScriptRuntime> Runtime;

    // Identifies the scripted contact-classification entry point.
    // Non-owning pointer borrows a function retained by Runtime.
    AngelScript::asIScriptFunction* ContactRule = nullptr;

    // Identifies the scripted replacement-policy entry point.
    // Non-owning pointer borrows a function retained by Runtime.
    AngelScript::asIScriptFunction* ReplacementRule = nullptr;
};
