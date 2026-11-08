#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"

namespace AngelScript
{
    class asIScriptEngine;
    class asIScriptModule;
    class asIScriptContext;
    class asIScriptFunction;
    struct asSMessageInfo;
}

/** Owns one compiled script and its reusable game-thread execution context. */
class FFeatureScriptRuntime
{
public:
    // Registers bridge types and compiles the named script from the game-logic directory.
    // FileName and TFunctionRef are borrowed only during construction; the callback avoids allocation and the runtime owns the engine/context.
    explicit FFeatureScriptRuntime(const TCHAR* FileName,
        TFunctionRef<void(AngelScript::asIScriptEngine&)> RegisterTypes = [](AngelScript::asIScriptEngine&) {});

    // Releases the context before its script engine.
    // No pointer or reference parameters; disposes exclusively owned native resources.
    ~FFeatureScriptRuntime();

    // Prevents sharing ownership through a shallow runtime copy.
    // The borrowed source reference cannot be copied; native resources have one owner.
    FFeatureScriptRuntime(const FFeatureScriptRuntime&) = delete;

    // Prevents replacing ownership through a shallow runtime assignment.
    // The source and return references are unavailable because assignment is deleted.
    FFeatureScriptRuntime& operator=(const FFeatureScriptRuntime&) = delete;

    // Resolves an entry point required by its focused C++ bridge.
    // Declaration borrows an ANSI string; the returned function reference belongs to the module.
    AngelScript::asIScriptFunction& FindFunction(const char* Declaration) const;

    // Selects a function and exposes its arguments for the next synchronous call.
    // Function and the returned context are borrowed native references owned by this runtime.
    AngelScript::asIScriptContext& Prepare(AngelScript::asIScriptFunction& Function);

    // Executes the prepared function and reports script exceptions.
    // No pointer or reference parameters or return value; the reusable context retains the result.
    void Execute();

private:
    // Routes script compilation messages to the feature log.
    // Message is a borrowed SDK pointer; UserData is an unused non-owning callback pointer.
    static void LogScriptMessage(const AngelScript::asSMessageInfo* Message, void* UserData);

    // Owns the script compiler and its module.
    // Native reference-counted pointer is released explicitly by the destructor.
    AngelScript::asIScriptEngine* Engine = nullptr;

    // Retains the compiled script's function definitions.
    // Non-owning native pointer borrows a module retained by Engine.
    AngelScript::asIScriptModule* Module = nullptr;

    // Reuses one execution context for synchronous rule calls.
    // Native reference-counted pointer is released before Engine.
    AngelScript::asIScriptContext* Context = nullptr;
};
