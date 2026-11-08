#include "FeatureLevelRules.h"
#include "Containers/StringConv.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TestFeatures.h"
#include <angelscript.h>

static_assert(ANGELSCRIPT_VERSION == FEATURE_ANGELSCRIPT_VERSION, "AngelScript header must match the pinned library version");

void UFeatureLevelRules::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ScriptEngine = AngelScript::asCreateScriptEngine();
    if (!ScriptEngine)
    {
        UE_LOG(LogTestFeatures, Fatal, TEXT("FeatureLevelRules: Could not create the AngelScript engine"));
    }
    ScriptEngine->SetMessageCallback(AngelScript::asFUNCTION(LogScriptMessage), nullptr, AngelScript::asCALL_CDECL);

    const FString ScriptPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source/TestFeatures/Features_GameLogic/FeatureLevelRules.as"));
    FString Source;
    if (!FFileHelper::LoadFileToString(Source, *ScriptPath))
    {
        UE_LOG(LogTestFeatures, Fatal, TEXT("FeatureLevelRules: Could not read %s"), *ScriptPath);
    }
    AngelScript::asIScriptModule* Module = ScriptEngine->GetModule("FeatureLevelRules", AngelScript::asGM_ALWAYS_CREATE);
    const FTCHARToUTF8 Utf8Source(*Source);
    Module->AddScriptSection("FeatureLevelRules.as", Utf8Source.Get(), Utf8Source.Length());
    if (Module->Build() < 0)
    {
        UE_LOG(LogTestFeatures, Fatal, TEXT("FeatureLevelRules: Script compilation failed; see diagnostics above"));
    }
    WinRule = Module->GetFunctionByDecl("int PlayerContactsToWin(int, int, int)");
    LossRule = Module->GetFunctionByDecl("int SessionBouncesToLose(int, int, int, int)");
    if (!WinRule || !LossRule)
    {
        UE_LOG(LogTestFeatures, Fatal, TEXT("FeatureLevelRules: Required threshold functions are missing"));
    }
    Context = ScriptEngine->CreateContext();
}

void UFeatureLevelRules::Deinitialize()
{
    Context->Release();
    ScriptEngine->ShutDownAndRelease();
    Super::Deinitialize();
}

void UFeatureLevelRules::LogScriptMessage(const AngelScript::asSMessageInfo* Message, void*)
{
    UE_LOG(LogTestFeatures, Display, TEXT("FeatureLevelRules: %s(%d,%d): %s"),
        UTF8_TO_TCHAR(Message->section), Message->row, Message->col, UTF8_TO_TCHAR(Message->message));
}

int32 UFeatureLevelRules::Execute(AngelScript::asIScriptFunction& Function, TConstArrayView<int32> Arguments) const
{
    Context->Prepare(&Function);
    for (int32 Index = 0; Index < Arguments.Num(); ++Index)
    {
        Context->SetArgDWord(Index, Arguments[Index]);
    }
    if (Context->Execute() != AngelScript::asEXECUTION_FINISHED)
    {
        UE_LOG(LogTestFeatures, Fatal, TEXT("FeatureLevelRules: %s failed: %s"),
            UTF8_TO_TCHAR(Function.GetName()), UTF8_TO_TCHAR(Context->GetExceptionString()));
    }
    return static_cast<int32>(Context->GetReturnDWord());
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
