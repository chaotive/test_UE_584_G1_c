# Crazy balls

**Features test on Unreal 5.8.4**

Crazy balls is a third-person ball-contact game. Move around the arena and intercept the bouncing balls before too many of their other impacts are counted. Each round starts with five balls launched from random positions in random directions.

![Crazy balls logo](Content/FeatureBrand/FeatureCrazyBallsLogo.png)

![Illustration of a player approaching bouncing balls in a training arena](docs/images/feature-rules-illustration.png)

*AI-generated illustration of the game's premise. This is concept art, not an in-game capture.*

## Jump into the arena

Choose **New Game** for a fresh game at the configured initial level (level 1 by default) or **Continue on level N** to play your saved level. Use **WASD** to move, the **mouse** to look, and **Space** to jump. Run into a ball to score a player contact: that ball is retired and another launches into the arena.

## Start fresh or pick up your count

The startup menu shows your saved level, total bounces, session count, and the latest session’s result and counts. The pause menu shows your last saved bounce total. On the startup menu, **Esc** exits. When the slot is empty, the menu shows **No saved game.** and disables **Continue**. Choose **Delete Save** on the startup menu to clear the slot; **New Game** also clears it before starting.

![Crazy balls startup menu](docs/images/crazy-startup.png)

*November 7, 2026: the startup menu shows the logo, subtitle, saved level, and bounce total.*

## How to play

Each player-ball contact also doubles your configured base movement speed for five seconds by default. Another contact refreshes the boost, and pausing the game pauses its countdown. Optional stacking adds another bonus for each contact and resets the shared timer; with the default 2× bonus, consecutive contacts produce 2×, 3×, and 4× base speed until it expires.

Each session is a race between two counters:

1. **Make player contacts.** Move or jump into the balls. Each registered player-ball contact increases your win counter. The ball disappears and another is launched, giving you another chance to score.
2. **Keep an eye on other bounces.** When a ball hits the floor, a wall, or another ball with enough physical impulse to register a bounce, the loss counter rises. Player contacts are counted separately. Two balls hitting each other can each register a bounce.
3. **Reach the win target first.** Reaching the player-contact target ends the session in Victory. Reaching the non-player bounce limit ends it in Defeat. The top-right HUD shows the current level and both counts against their limits.

Each ball has its own lifetime: after its configured number of non-player bounces (five by default), it returns to the pool. A ball that touches the player is also retired immediately. In either case, a replacement is scheduled while the session is active.

### Levels and results

By default, every new game begins at level 1; the initial level is configurable in the game mode. Winning advances the saved level by one; losing keeps the same level. Each new session resets its two objective counters, while the saved aggregate bounce total and session history continue across sessions.

| Level | Player contacts to win | Non-player bounces to lose |
| --- | ---: | ---: |
| 1 | 10 | 100 |
| 2 | 11 | 99 |
| 3 | 12 | 98 |
| 50 | 59 | 51 |

The defaults follow `10 + (level - 1)` for the win target and `max(50, 100 - (level - 1))` for the loss limit. The game mode can change the base limits, per-level adjustments, and minimum loss limit. The default loss limit starts at 100 and decreases by one per level until it reaches 50. On **Victory**, the game pauses, records the result, advances the saved level, and requests a save. Choose **Continue** to start the next level. On **Defeat**, the game records and saves the result without advancing; choose **Retry** to start another session at that level, or **Exit** to save and quit.

### Menus, pause, and saves

**New Game** deletes the current save and begins at the configured initial level with a fresh history. **Continue on level N** loads a readable save and starts its saved level. **Delete Save** removes the slot without starting a session; when there is no save, Continue is disabled and the menu displays **No saved game.** **Exit** closes the game, and **Esc** on the initial menu does the same.

During a session, **Esc** opens the pause menu. Choose **Resume Game** or press **Esc** again to continue the same session; **Main Menu** returns to the startup screen, and **Exit** saves and quits. Saving or loading pauses play during I/O. In non-Shipping builds, the debug panel also offers **Save game** and **Save and quit** and shows save diagnostics.

![Crazy balls pause menu](docs/images/crazy-pause.png)

*November 7, 2026: the pause menu, opened through the editor console, shows the shared logo and session controls.*

The save contains the current level, aggregate non-player bounce total, and a chronological session history with start times, counters, and results. The gameplay HUD's counters are for the current session; the debug HUD's total includes the saved history. A result screen requests a save automatically.

The renamed game uses a `CrazyBalls` save slot. Saves from the former `FeatureSandbox` slot are not loaded by this version.

## Development

### Build and run

1. Open `test_UE_584_G1_c.uproject` in Unreal Engine 5.8 and build the editor target.
2. Open the Third Person map and press **Play**.
3. In a non-Shipping build, enter `Feature.NewGame` in the console at the startup menu to start immediately. In Play In Editor, use the Output Log command field. [Console command implementation](Source/test_UE_584_G1_c/FeatureConsoleCommands.cpp)

### Smoke checks

After building the Development Editor target, run `./Run-FeatureSmoke.ps1` in PowerShell. It invokes Unreal's Gauntlet editor automation runner for the `Feature.Smoke` checks: menu definitions, session state, ball-pool reuse and player-contact replacement, loss-limit progression and minimum, save serialization, and temporary player-speed boosts including timer expiry and stacking. Reports and logs are stored under `Intermediate/FeatureSmoke`. These checks do not replace a playthrough or exercise the rendered UI.

### Package for Win64

[Build-Win64.ps1](Build-Win64.ps1) generates project files, then packages Development, DebugGame, and Shipping together. It discovers the project's Unreal Engine version through the Windows installation registry, falling back to `D:\Program Files\Epic Games\UE_<version>`. Project and archive paths are relative to the script. Project-file generation must succeed before packaging starts.

```powershell
.\Build-Win64.ps1
```

The default archive ID is `Dev`, so repeated calls reuse `Builds\Dev`. After packaging, the script copies the latest AutomationTool `Log.txt` for this project's current run to `Build.log` in that archive, including failed packaging runs that produced a log. Reusing an ID replaces its attached log. Supply an ID to use a different archive directory:

```powershell
.\Build-Win64.ps1 -Id '1'
```

DebugGame supports debugging project C++. Each archive contains a `Windows` directory with configuration-specific launchers. Keep the staged files together when launching any of the three builds.

The asset manager registers the `GameFeatureData` cook rule required by the engine's Game Features subsystem. [Project settings](Config/DefaultGame.ini)

### Arena collision bounds

The game-mode defaults expose the base player speed multiplier, contact speed multiplier, stacking option, and contact boost duration under `Feature Player Speed`. They also expose the initial level, initial ball count, per-level objective adjustments, rebounds before retirement, ball size, and initial launch-speed multiplier. The pool prewarms one ball per initial ball; its default size and the initial ball count are both five. A size or speed multiplier of 1 preserves the default ball size or launch speed.

Gameplay creates one `AFeatureLevelBounds` actor with six wireframe collision faces visible in game. The initial interior spans 120 × 120 × 60 meters around a center 20 meters above the world origin. Edit `Feature Bounds` in the game-mode defaults to change its center and half-size. The faces block physics balls. Balls return to the pool after their configured rebound limit or player contact and are replaced while the session is active.

`UFeatureGameplaySession` is a world subsystem that organizes `OnGameplaySessionStart`, `OnGameplaySessionResume`, `OnGameplaySessionPaused`, and `OnGameplaySessionEnded`. Its start workflow creates the enclosure before the initial ball volley. The game mode observes lifecycle events to spawn the player and present screens. Other features can access the session through `World.GetSubsystem<UFeatureGameplaySession>()` and subscribe to its lifecycle events.

### Session results

A victory or defeat pauses gameplay, saves the session, and presents the matching fullscreen result. **Continue** starts the next saved level after victory. **Retry** reloads the arena and starts a new session after defeat.

### Debug HUD and save diagnostics

The non-Shipping debug HUD shows the live bounce count, the save slot and file path, and the I/O history. Its **Save game** button writes a snapshot while keeping the session open. **Save and quit** exits after a successful write.

Gameplay remains paused until asynchronous save or load I/O finishes.

### Architecture

Three square diagrams summarize responsibilities, dependencies, and patterns. Solid arrows show calls or data flow; the dashed arrow shows state observation. Expand the class references for full names and base types. These are architecture illustrations, not runtime captures.

#### Gameplay architecture

![Square gameplay architecture diagram: session, arena, pool, ball, state bridge and shared state.](docs/images/feature-gameplay-flow.png)

Object pooling reuses actors. Observer events connect balls to the state bridge and shared state without a direct ball-to-state dependency. Both player contact and the rebound limit retire a ball and schedule a replacement while playing.

<details>
<summary>Class reference</summary>

| Diagram role | Class | Base or primary consumer |
| --- | --- | --- |
| Gameplay session | `UFeatureGameplaySession` | Extends `UWorldSubsystem` |
| Arena bounds | `AFeatureLevelBounds` | Extends `AActor` |
| Ball pool | `FFeatureBallSpawner` | Creates `AFeatureBall` for `UFeatureGameplaySession` |
| Physics ball | `AFeatureBall` | Extends `AStaticMeshActor` |
| State event bridge | `UFeatureGameStateController` | Extends `UObject` |
| Game state | `AFeatureGameState` | Extends `AGameStateBase` |

</details>

Source: [session](Source/TestFeatures/Features/GameWorkflow/FeatureGameplaySession.cpp) · [pool](Source/TestFeatures/Features_NonUE/FeatureBallSpawner.cpp) · [ball](Source/TestFeatures/Features/Gameplay/FeatureBall.cpp) · [state bridge](Source/TestFeatures/Features/Gameplay/FeatureGameStateController.cpp).

#### UI architecture

![Square UI architecture diagram: declarative menu definitions, shared menu, gameplay HUD and composed debug panels.](docs/images/feature-ui-flow.png)

Declarative views share one menu implementation. The game mode manages menus; the player controller manages HUD lifetime. Widgets own their Observer bindings.

<details>
<summary>Class reference</summary>

| Diagram role | Class | Base or primary consumer |
| --- | --- | --- |
| Menu definitions | `FFeatureMenuViews` | Creates `FFeatureMenuView` values for `UFeatureMainMenu` |
| Shared menu | `UFeatureMainMenu` | Extends `UUserWidget` |
| Gameplay HUD | `UFeatureGameplayHUD` | Extends `UUserWidget` |
| Debug HUD | `UFeatureDebugHUD` | Extends `UUserWidget` |
| Ball panel | `UFeatureBallPanel` | Extends `UUserWidget` |
| Save panel | `UFeatureSaveGamePanel` | Extends `UUserWidget` |

</details>

Source: [views](Source/TestFeatures/Features/UI/Views/FeatureMenuViews.cpp) · [menu](Source/TestFeatures/Features/UI/Archetypes/FeatureMainMenu.cpp) · [HUDs](Source/TestFeatures/Features/UI/Archetypes/FeatureGameplayHUD.cpp) · [panels](Source/TestFeatures/Features/UI/Widgets/FeatureSaveGamePanel.cpp).

#### Session and persistence architecture

![Square session architecture diagram: game mode, gameplay session, progress state, save coordinator, storage and snapshot.](docs/images/feature-workflow-flow.png)

An enum-based state machine controls round transitions in the world subsystem. The game-instance subsystem coordinates persistence; storage creates a value snapshot from current state. The game state requests save/load operations, and the save subsystem publishes restored data and I/O status.

<details>
<summary>Class reference</summary>

| Diagram role | Class | Base or primary consumer |
| --- | --- | --- |
| Game mode | `Atest_UE_584_G1_cGameMode` | Extends `AGameModeBase` |
| Gameplay session | `UFeatureGameplaySession` | Extends `UWorldSubsystem` |
| Game state | `AFeatureGameState` | Extends `AGameStateBase` |
| Save coordinator | `UFeatureSaveGameSubsystem` | Extends `UGameInstanceSubsystem` |
| Save storage | `FFeatureSaveGameStorage` | Creates `UFeatureSaveGame` for `UFeatureSaveGameSubsystem` |
| Save snapshot | `UFeatureSaveGame` | Extends `USaveGame` |

</details>

Source: [game mode](Source/test_UE_584_G1_c/test_UE_584_G1_cGameMode.cpp) · [save coordinator](Source/TestFeatures/Features/GameWorkflow/FeatureSaveGameSubsystem.cpp) · [storage](Source/TestFeatures/Features_NonUE/FeatureSaveGameStorage.cpp) · [snapshot](Source/TestFeatures/Features/GameWorkflow/FeatureSaveGame.h).

The `TestFeatures` runtime module houses gameplay, UI, and persistence. Unreal and Blueprint provide the framework and supplementary assets; AngelScript remains a project goal.

Within `TestFeatures/Features`, `Gameplay` contains balls, bounds, and bounce state. `GameWorkflow` contains session and persistence code. `UI` contains menus and HUD panels. Plain C++ spawning and storage helpers live in the sibling `Features_NonUE` folder.

- [Gameplay systems and HUD](Source/TestFeatures/Features/README.md)
- [Runtime helpers](Source/TestFeatures/Features_NonUE/README.md)
- [Project objectives and technical requirements](AGENTS.MD)
- [Completed tasks and capture index](CHANGES.md)

### Local Unreal Editor MCP

For explicitly requested editor work, open `test_UE_584_G1_c.uproject` normally. The project enables Epic's built-in MCP and toolset plugins for editor targets and starts the local server automatically. To turn off automatic startup for your local editor, clear **Auto Start Server** under **Edit → Editor Preferences → Model Context Protocol**.

The trusted project's `.codex/config.toml` points Codex at `http://127.0.0.1:8000/mcp`. If the endpoint is unavailable and Unreal Editor is closed, open the project and wait for the editor to finish loading and the local server to respond. If the current chat does not expose the Unreal tools once the server responds, start a new Codex chat in this project with the editor still open.

### Game evolution

![Earlier startup menu](docs/images/current-startup.png)

*November 7, 2026, before the Crazy balls rename: the startup menu showed the previous title and saved total.*

![Earlier gameplay HUD](docs/images/current-gameplay.png)

*November 7, 2026, before the rename: gameplay showed level-one objective counters and the earlier debug HUD branding.*

![Historical loading HUD](docs/images/in-game-loading-hud.png)

*November 1, 2026, before the main menu: an offscreen test showed load progress and a restored total of 96 bounces.*

![First startup menu](docs/images/task-18-main-menu.jpg)

*Task 18, November 7, 2026: the first startup menu offered New Game and Continue; the saved-total summary and Delete Save action came later.*
