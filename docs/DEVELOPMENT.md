# Development guide

This guide covers running and testing Crazy balls, changing its implementation, and using the project tools. Commands run from the project root. See the [main README](../README.md) for the game and high-level architecture.

## Topic index

- **Run, test, and deliver:** [Build and run](#build-and-run) · [Smoke checks](#smoke-checks) · [Package for Win64](#package-for-win64) · [Unreal Editor MCP](#local-unreal-editor-mcp).
- **Gameplay and diagnostics:** [Game-mode configuration](#game-mode-configuration) · [Arena collision bounds](#arena-collision-bounds) · [Session results](#session-results) · [Debug HUD and save diagnostics](#debug-hud-and-save-diagnostics).
- **Change game logic:** [Game-logic workflow](../Source/TestFeatures/Features_GameLogic/README.GAMELOGIC.md) · [Script loading](#angelscript-rules) · [Library versioning and updates](../Source/ThirdParty/FeatureAngelScriptRuntime/README.ANGELSCRIPT.md).
- **Implementation references:** [Feature walkthrough](../Source/TestFeatures/Features/README.FEATURES.md) · [Non-UE helpers](../Source/TestFeatures/Features_NonUE/README.NON_UE.md) · [Performance review](PERFORMANCE_IDEAS.md).

## Run, test, and deliver

### Build and run

1. Open `test_UE_584_G1_c.uproject` in Unreal Engine 5.8 and build the editor target.
2. Open the Third Person map and press **Play**.
3. In a non-Shipping build, enter `Feature.NewGame` in the console at the startup menu to start immediately. In Play In Editor, use the Output Log command field. [Console command implementation](../Source/test_UE_584_G1_c/FeatureConsoleCommands.cpp)

### Smoke checks

After building the Development Editor target, run `./Run-FeatureSmoke.ps1` in PowerShell. It invokes Unreal's Gauntlet editor automation runner for the `Feature.Smoke` checks: menu definitions, session state, ball-pool reuse and player-contact replacement, loss-limit progression and minimum, save serialization, and temporary player-speed boosts including timer expiry and stacking. Reports and logs are stored under `Intermediate/FeatureSmoke`. These checks do not replace a playthrough or exercise the rendered UI.

The latest recorded Gauntlet attempt failed during editor startup and executed no checks; see the [tracked startup issue](../AGENTS.MD#bugs).

### Package for Win64

[Build-Win64.ps1](../Build-Win64.ps1) generates project files, then packages Development, DebugGame, and Shipping together. It discovers the project's Unreal Engine version through the Windows installation registry, falling back to `D:\Program Files\Epic Games\UE_<version>`. Project and archive paths are relative to the script. Project-file generation must succeed before packaging starts.

```powershell
.\Build-Win64.ps1
```

The default archive ID is `Dev`, so repeated calls reuse `Builds\Dev`. After packaging, the script copies the latest AutomationTool `Log.txt` for this project's current run to `Build.log` in that archive, including failed packaging runs that produced a log. Reusing an ID replaces its attached log. Supply an ID to use a different archive directory:

```powershell
.\Build-Win64.ps1 -Id '1'
```

DebugGame supports debugging project C++. Each archive contains a `Windows` directory with configuration-specific launchers. Keep the staged files together when launching any of the three builds.

The asset manager registers the `GameFeatureData` cook rule required by the engine's Game Features subsystem. [Project settings](../Config/DefaultGame.ini)

### Local Unreal Editor MCP

For explicitly requested editor work, open `test_UE_584_G1_c.uproject` normally. The project enables Epic's built-in MCP and toolset plugins for editor targets and starts the local server automatically. To turn off automatic startup for your local editor, clear **Auto Start Server** under **Edit → Editor Preferences → Model Context Protocol**.

The trusted project's `.codex/config.toml` points Codex at `http://127.0.0.1:8000/mcp`. If the endpoint is unavailable and Unreal Editor is closed, open the project and wait for the editor to finish loading and the local server to respond. If the current chat does not expose the Unreal tools once the server responds, start a new Codex chat in this project with the editor still open.

## Gameplay and diagnostics

### Game-mode configuration

The game-mode defaults expose the base player speed multiplier, contact speed multiplier, stacking option, and contact boost duration under `Feature Player Speed`. They also expose the initial level, initial ball count, per-level objective adjustments, rebounds before retirement, ball size, and initial launch-speed multiplier. The pool prewarms one ball per initial ball; the C++ default initial count is five. Blueprint game-mode defaults can override these values: November 8's capture used two balls, ten rebounds per ball, and level-one targets of four contacts and 100 bounces. Check the level's active game mode when changing settings. A size or speed multiplier of 1 preserves the default ball size or launch speed. The [game-mode declarations](../Source/test_UE_584_G1_c/test_UE_584_G1_cGameMode.h) define the exposed properties.

### Arena collision bounds

Gameplay creates one `AFeatureLevelBounds` actor with six wireframe collision faces visible in game. The C++ defaults span 120 × 120 × 60 meters around a center 20 meters above the world origin. Level overrides can differ: November 8's runtime log recorded half-extents of 19.2 × 19.2 × 30 meters, giving a 38.4 × 38.4 × 60-meter interior. Edit `Feature Bounds` in the active game-mode defaults to change its center and half-size. The faces block physics balls. Balls return to the pool after their configured rebound limit or player contact and are replaced while the session is active.

### Session results

`UFeatureGameplaySession` is a world subsystem that organizes `OnGameplaySessionStart`, `OnGameplaySessionResume`, `OnGameplaySessionPaused`, and `OnGameplaySessionEnded`. Its start workflow creates the enclosure before the initial ball volley. The game mode observes lifecycle events to spawn the player and present screens. Other features can access the session through `World.GetSubsystem<UFeatureGameplaySession>()` and subscribe to its lifecycle events.

A victory or defeat pauses gameplay, saves the session, and presents the matching fullscreen result. **Continue** starts the next saved level after victory. **Retry** reloads the arena and starts a new session after defeat.

### Debug HUD and save diagnostics

The non-Shipping debug HUD shows the live bounce count, the save slot and file path, and the I/O history. Its **Save game** button writes a snapshot while keeping the session open. **Save and quit** exits after a successful write.

Gameplay remains paused until asynchronous save or load I/O finishes.

## Change game logic

### AngelScript rules

The [game-logic guide](../Source/TestFeatures/Features_GameLogic/README.GAMELOGIC.md) covers rule changes and ownership in `Features_GameLogic`, where `.as` source and its project-authored C++ bridges stay together. [FeatureLevelRules.as](../Source/TestFeatures/Features_GameLogic/FeatureLevelRules.as) is the only implementation of the win/loss threshold formulas. C++ owns the game-mode configuration and saved progress, passes integer values into the script, and uses the returned limits for gameplay and the HUD.

The [rules world subsystem](../Source/TestFeatures/Features_GameLogic/FeatureLevelRules.cpp) reads and compiles the saved `.as` file once per world. For script-only changes, stop PIE, edit and save the script, then start PIE again. The new PIE world creates a fresh host that reads the latest file from disk. This requires neither a C++ game rebuild nor an editor restart; the AngelScript compiler handles the updated rules when the world initializes. Script errors are logged and stop execution rather than selecting a duplicate C++ fallback. The build stages `.as` files as UFS runtime dependencies for packaged games.

The project links a prebuilt Win64 AngelScript static library through an external Unreal module. The [dependency manifest](../Source/ThirdParty/FeatureAngelScriptRuntime/Dependency.json) pins its version, archive checksum, toolchain and build options. The [library README](../Source/ThirdParty/FeatureAngelScriptRuntime/README.ANGELSCRIPT.md) describes the supplied artifacts and update process. SDK source and build intermediates stay in the ignored `.dependencies/AngelScript/<version>` cache; normal Unreal builds only validate and link the library.
