# AngelScript runtime dependency

[Dependency.json](Dependency.json) pins the source version, official archive URL, SHA-256, MSVC toolchain, Windows SDK and compilation options. The external Unreal module reads that version directly to select its library and header directory.

The supplied artifact is **AngelScript 2.38.0, Win64 x64, Release, dynamic CRT (`/MD`)**. It supports this project's Development, DebugGame and Shipping builds. Other platforms, architectures and debug-CRT builds need their own library variant and are explicitly rejected.

## Files and source cache

Artifact paths below are relative to `Source/ThirdParty/FeatureAngelScriptRuntime/`.

- `2.38.0/lib/Win64/Release/angelscript.lib`: the compiled static library to check in.
- `2.38.0/include/angelscript.h`: the unmodified public API header required by C++ consumers.
- `2.38.0/BuildInfo.json`: records the manifest, library and header hashes and build toolchain.
- `2.38.0/LICENSE.txt`: upstream license and attribution.
- `/.dependencies/AngelScript/2.38.0/`: ignored cache containing the verified official SDK archive, extracted source and build intermediates.

Ordinary Unreal builds only validate and link the supplied library. They do not download or compile the SDK, and a clean checkout does not require the SDK cache. The module checks the artifact hashes against `BuildInfo.json`; the C++ adapter also checks the header version at compile time.

The library uses `AS_USE_NAMESPACE`, `AS_MAX_PORTABILITY` and `AS_NO_EXCEPTIONS`. No Unreal plugin, engine fork or AngelScript DLL is required. Our script API passes integer values; these build options are sufficient for that interface.

## Build or restore the source

From the project root, using PowerShell:

```powershell
.\Source\ThirdParty\FeatureAngelScriptRuntime\Build-FeatureAngelScript.ps1
.\Source\ThirdParty\FeatureAngelScriptRuntime\Build-FeatureAngelScript.ps1 -FetchOnly
.\Source\ThirdParty\FeatureAngelScriptRuntime\Build-FeatureAngelScript.ps1 -Rebuild
```

The first command reuses matching artifacts or builds them when missing or stale. `-FetchOnly` restores the verified source cache without compiling. `-Rebuild` compiles from the pinned archive and refreshes the library, header, license and build information. Initial downloads require network access; subsequent rebuilds use the verified cached archive. Missing pinned tools cause an error rather than selecting another compiler silently.

## Update the dependency

1. Select an upstream release from the [official downloads](https://www.angelcode.com/angelscript/downloads.html).
2. Update `Version`, `SourceUrl` and `SourceSha256` in `Dependency.json`. Update toolchain versions or options there only when intentional.
3. Run `Build-FeatureAngelScript.ps1 -Rebuild`. Check the new version directory and `BuildInfo.json`.
4. Compile the Unreal Development Editor and Development game targets. Verify script compilation and binding compatibility; perform runtime checks when requested.
5. Update this guide's artifact version and paths, then review the library, public header, license, manifest and build information for the next commit. Remove the old version directory when no longer needed. Never add the `/.dependencies/` cache.

The SDK implementation remains unmodified in the ignored cache. The [game-logic guide](../../TestFeatures/Features_GameLogic/README.GAMELOGIC.md) covers rule development and the project-specific C++ bridge.
