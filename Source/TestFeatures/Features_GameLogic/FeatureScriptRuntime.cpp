#include "FeatureScriptRuntime.h"
#include "Containers/StringConv.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TestFeatures.h"
#include <angelscript.h>

static_assert(ANGELSCRIPT_VERSION == FEATURE_ANGELSCRIPT_VERSION, "AngelScript header must match the pinned library version");

FFeatureScriptRuntime::FFeatureScriptRuntime(const TCHAR* FileName)
{
    Engine = AngelScript::asCreateScriptEngine();
    if (!Engine)
    {
        UE_LOG(LogTestFeatures, Fatal, TEXT("FeatureScriptRuntime: Could not create the AngelScript engine"));
    }
    Engine->SetMessageCallback(AngelScript::asFUNCTION(LogScriptMessage), nullptr, AngelScript::asCALL_CDECL);

    const FString ScriptPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source/TestFeatures/Features_GameLogic"), FileName);
    FString Source;
    if (!FFileHelper::LoadFileToString(Source, *ScriptPath))
    {
        UE_LOG(LogTestFeatures, Fatal, TEXT("FeatureScriptRuntime: Could not read %s"), *ScriptPath);
    }
    Module = Engine->GetModule("FeatureRules", AngelScript::asGM_ALWAYS_CREATE);
    const FTCHARToUTF8 Utf8Source(*Source);
    const FTCHARToUTF8 Utf8FileName(FileName);
    Module->AddScriptSection(Utf8FileName.Get(), Utf8Source.Get(), Utf8Source.Length());
    if (Module->Build() < 0)
    {
        UE_LOG(LogTestFeatures, Fatal, TEXT("FeatureScriptRuntime: Compilation failed for %s; see diagnostics above"), FileName);
    }
    Context = Engine->CreateContext();
}

FFeatureScriptRuntime::~FFeatureScriptRuntime()
{
    Context->Release();
    Engine->ShutDownAndRelease();
}

AngelScript::asIScriptFunction& FFeatureScriptRuntime::FindFunction(const char* Declaration) const
{
    AngelScript::asIScriptFunction* Function = Module->GetFunctionByDecl(Declaration);
    if (!Function)
    {
        UE_LOG(LogTestFeatures, Fatal, TEXT("FeatureScriptRuntime: Required function is missing: %s"), UTF8_TO_TCHAR(Declaration));
    }
    return *Function;
}

AngelScript::asIScriptContext& FFeatureScriptRuntime::Prepare(AngelScript::asIScriptFunction& Function)
{
    Context->Prepare(&Function);
    return *Context;
}

void FFeatureScriptRuntime::Execute()
{
    if (Context->Execute() != AngelScript::asEXECUTION_FINISHED)
    {
        UE_LOG(LogTestFeatures, Fatal, TEXT("FeatureScriptRuntime: Script execution failed: %s"),
            UTF8_TO_TCHAR(Context->GetExceptionString()));
    }
}

void FFeatureScriptRuntime::LogScriptMessage(const AngelScript::asSMessageInfo* Message, void*)
{
    UE_LOG(LogTestFeatures, Display, TEXT("FeatureScriptRuntime: %s(%d,%d): %s"),
        UTF8_TO_TCHAR(Message->section), Message->row, Message->col, UTF8_TO_TCHAR(Message->message));
}
