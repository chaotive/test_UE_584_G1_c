# AngelScript runtime dependency

[Dependency.json](Dependency.json) pins the source version, official archive URL, SHA-256, MSVC toolchain, Windows SDK and compilation options. The external Unreal module reads that version directly to select its library and header directory.

The supplied artifact is **AngelScript 2.38.0, Win64 x64, Release, dynamic CRT (`/MD`)**. It supports this project's Development, DebugGame and Shipping builds. Other platforms, architectures and debug-CRT builds need their own library variant and are explicitly rejected.

## Files and source cache

Artifact paths below are relative to `Source/ThirdParty/FeatureAngelScriptRuntime/`.

- `2.38.0/lib/Win64/Release/angelscript.lib`: the compiled static library to check in.
- `2.38.0/include/angelscript.h`: the unmodified public API header required by C++ consumers.
- `2.38.0/BuildInfo.json`: records the manifest, library and header hashes and build toolchain.
- `2.38.0/LICENSE.txt`: upstream license and attribution.
- `2.38.0/tools/Win64/asrun.exe` is the SDK's standalone script runner for unit tests.
- `/.dependencies/AngelScript/2.38.0/`: ignored cache containing the verified official SDK archive, extracted source and build intermediates.

Ordinary Unreal builds only validate and link the supplied library. They do not download or compile the SDK, and a clean checkout does not require the SDK cache. The module checks the artifact hashes against `BuildInfo.json`; the C++ adapter also checks the header version at compile time.

The library uses `AS_USE_NAMESPACE`, `AS_MAX_PORTABILITY` and `AS_NO_EXCEPTIONS`. No Unreal plugin, engine fork or AngelScript DLL is required. The script API passes integers, floats, booleans, and registered native enums through execution-context argument and return-value methods. It exposes no Unreal objects to scripts.

## Build or restore the source

From the project root, using PowerShell:

```powershell
.\Source\ThirdParty\FeatureAngelScriptRuntime\Build-FeatureAngelScript.ps1
.\Source\ThirdParty\FeatureAngelScriptRuntime\Build-FeatureAngelScript.ps1 -FetchOnly
.\Source\ThirdParty\FeatureAngelScriptRuntime\Build-FeatureAngelScript.ps1 -Rebuild
```

The first command reuses matching artifacts or builds them when missing or stale. `-FetchOnly` restores the verified source cache without compiling. `-Rebuild` compiles from the pinned archive and refreshes the library, header, license and build information. Initial downloads require network access; subsequent rebuilds use the verified cached archive. Missing pinned tools cause an error rather than selecting another compiler silently.

## Development runner

`asrun.exe` is built from the unmodified SDK projects in the same verified 2.38.0 archive, in Release x64 with the static CRT (`/MT`). It embeds the stock AngelScript library because its native callbacks require support that the game's `AS_MAX_PORTABILITY` build omits. The binary's SHA-256 is `50F725523896C14C898EA75793282BF4A6CE251955E516E9B3C3A99C38DEE6FC`. [SDK license](2.38.0/LICENSE.txt) · [Standalone unit tests](../../../docs/DEVELOPMENT.md#standalone-unit-tests)

To rebuild the tool, verify the SDK archive against `SourceSha256` in `Dependency.json` and extract it into `.dependencies/AngelScript/2.38.0/asrun-sdk/` with `sdk/` directly inside. Run these commands from the repository root in a Visual Studio Developer Command Prompt.

```cmd
set "ASRUN_SDK=.dependencies\AngelScript\2.38.0\asrun-sdk\sdk"
set "ASRUN_BUILD_FLAGS=/t:Rebuild /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v145 /p:VCToolsVersion=14.50.35717 /p:WindowsTargetPlatformVersion=10.0.22621.0"
msbuild "%ASRUN_SDK%\angelscript\projects\msvc2022\angelscript.vcxproj" %ASRUN_BUILD_FLAGS%
msbuild "%ASRUN_SDK%\samples\asrun\projects\msvc2022\asrun.vcxproj" %ASRUN_BUILD_FLAGS%
copy /Y "%ASRUN_SDK%\samples\asrun\bin\asrun.exe" "Source\ThirdParty\FeatureAngelScriptRuntime\2.38.0\tools\Win64\asrun.exe"
```

Record the new executable's SHA-256 here and run the unit suite after replacing it.

## Update the dependency

1. Select an upstream release from the [official downloads](https://www.angelcode.com/angelscript/downloads.html).
2. Update `Version`, `SourceUrl` and `SourceSha256` in `Dependency.json`. Update toolchain versions or options there only when intentional.
3. Run `Build-FeatureAngelScript.ps1 -Rebuild`. Check the new version directory and `BuildInfo.json`.
4. Compile the Unreal Development Editor and Development game targets. Verify script compilation and binding compatibility; perform runtime checks when requested.
5. Rebuild the development runner from the new SDK and place it in the versioned `tools/Win64` directory. The [unit test script](../../TestFeatures/Features_GameLogic/Tests/Run-FeatureUnitTests.ps1) reads the version from the manifest. Review the runner, library, public header, license, manifest and build information for the next commit. Remove the old version directory when no longer needed. Never add the `/.dependencies/` cache.

The SDK implementation remains unmodified in the ignored cache. The [game-logic guide](../../TestFeatures/Features_GameLogic/README.GAMELOGIC.md) covers rule development and the project-specific C++ bridge.
