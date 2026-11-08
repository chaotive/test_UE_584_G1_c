#include "FeatureLevelRules.h"
#include "Features/GameWorkflow/FeatureSaveGame.h"
#include <angelscript.h>

void UFeatureLevelRules::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Runtime = MakeUnique<FFeatureScriptRuntime>(TEXT("FeatureLevelRules.as"), RegisterTypes);
    WinRule = &Runtime->FindFunction("int PlayerContactsToWin(int, int, int)");
    LossRule = &Runtime->FindFunction("int SessionBouncesToLose(int, int, int, int)");
    OutcomeRule = &Runtime->FindFunction("SessionResult SessionOutcome(int, int, int, int)");
    ProgressionRule = &Runtime->FindFunction("int NextLevel(int, SessionResult)");
}

void UFeatureLevelRules::RegisterTypes(AngelScript::asIScriptEngine& Engine)
{
    Engine.RegisterEnum("SessionResult");
    Engine.RegisterEnumValue("SessionResult", "InProgress", static_cast<int>(EFeatureGameplaySessionResult::InProgress));
    Engine.RegisterEnumValue("SessionResult", "Victory", static_cast<int>(EFeatureGameplaySessionResult::Victory));
    Engine.RegisterEnumValue("SessionResult", "Defeat", static_cast<int>(EFeatureGameplaySessionResult::Defeat));
}

void UFeatureLevelRules::Deinitialize()
{
    Runtime.Reset();
    Super::Deinitialize();
}

int32 UFeatureLevelRules::Execute(AngelScript::asIScriptFunction& Function, TConstArrayView<int32> Arguments) const
{
    AngelScript::asIScriptContext& Context = Runtime->Prepare(Function);
    for (int32 Index = 0; Index < Arguments.Num(); ++Index)
    {
        Context.SetArgDWord(Index, Arguments[Index]);
    }
    Runtime->Execute();
    return static_cast<int32>(Context.GetReturnDWord());
}

int32 UFeatureLevelRules::PlayerContactsToWin(int32 Level, int32 BaseContacts, int32 IncreasePerLevel) const
{
    const int32 Arguments[] = { Level, BaseContacts, IncreasePerLevel };
    return Execute(*WinRule, Arguments);
}

int32 UFeatureLevelRules::SessionBouncesToLose(int32 Level, int32 BaseBounces, int32 DecreasePerLevel, int32 MinimumBounces) const
{
    const int32 Arguments[] = { Level, BaseBounces, DecreasePerLevel, MinimumBounces };
    return Execute(*LossRule, Arguments);
}

EFeatureGameplaySessionResult UFeatureLevelRules::SessionOutcome(int32 PlayerContacts, int32 Bounces, int32 WinLimit, int32 LossLimit) const
{
    const int32 Arguments[] = { PlayerContacts, Bounces, WinLimit, LossLimit };
    return static_cast<EFeatureGameplaySessionResult>(Execute(*OutcomeRule, Arguments));
}

int32 UFeatureLevelRules::NextLevel(int32 Level, EFeatureGameplaySessionResult Result) const
{
    const int32 Arguments[] = { Level, static_cast<int32>(Result) };
    return Execute(*ProgressionRule, Arguments);
}
