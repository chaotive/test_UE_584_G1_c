#include "FeatureBallRules.h"
#include "GameFramework/Pawn.h"
#include <angelscript.h>

void UFeatureBallRules::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Runtime = MakeUnique<FFeatureScriptRuntime>(TEXT("FeatureBallRules.as"), RegisterTypes);
    ContactRule = &Runtime->FindFunction("int EvaluateBallContact(bool, bool, int, int)");
    ReplacementRule = &Runtime->FindFunction("bool ShouldReplaceBall(bool)");
}

void UFeatureBallRules::RegisterTypes(AngelScript::asIScriptEngine& Engine)
{
    Engine.RegisterEnum("BallContactAction");
    Engine.RegisterEnumValue("BallContactAction", "None", static_cast<int>(EFeatureBallContactAction::None));
    Engine.RegisterEnumValue("BallContactAction", "CountBounce", static_cast<int>(EFeatureBallContactAction::CountBounce));
    Engine.RegisterEnumValue("BallContactAction", "Retire", static_cast<int>(EFeatureBallContactAction::Retire));
}

void UFeatureBallRules::Deinitialize()
{
    Runtime.Reset();
    Super::Deinitialize();
}

EFeatureBallContactAction UFeatureBallRules::EvaluateContact(AActor* OtherActor, bool bHasSolverImpulse, int32 BounceCount, int32 ReboundLimit) const
{
    const APawn* Pawn = Cast<APawn>(OtherActor);
    AngelScript::asIScriptContext& Context = Runtime->Prepare(*ContactRule);
    Context.SetArgByte(0, Pawn && Pawn->IsPlayerControlled());
    Context.SetArgByte(1, bHasSolverImpulse);
    Context.SetArgDWord(2, BounceCount);
    Context.SetArgDWord(3, ReboundLimit);
    Runtime->Execute();
    return static_cast<EFeatureBallContactAction>(Context.GetReturnDWord());
}

bool UFeatureBallRules::ShouldReplaceBall(bool bSessionPlaying) const
{
    AngelScript::asIScriptContext& Context = Runtime->Prepare(*ReplacementRule);
    Context.SetArgByte(0, bSessionPlaying);
    Runtime->Execute();
    return Context.GetReturnByte() != 0;
}
