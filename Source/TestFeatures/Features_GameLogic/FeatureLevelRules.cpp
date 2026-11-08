#include "FeatureLevelRules.h"
#include <angelscript.h>

void UFeatureLevelRules::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Runtime = MakeUnique<FFeatureScriptRuntime>(TEXT("FeatureLevelRules.as"));
    WinRule = &Runtime->FindFunction("int PlayerContactsToWin(int, int, int)");
    LossRule = &Runtime->FindFunction("int SessionBouncesToLose(int, int, int, int)");
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
