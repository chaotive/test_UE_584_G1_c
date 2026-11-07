# Feature Sandbox

Feature Sandbox is a third-person ball-contact game. Move around the arena and intercept the bouncing balls before too many of their other impacts are counted. Each round starts with five balls launched from random positions in random directions.

## Jump into the arena

Choose **New Game** for a fresh level-one game or **Continue on level N** to play your saved level. Use **WASD** to move, the **mouse** to look, and **Space** to jump. Run into a ball to score a player contact: that ball is retired and another launches into the arena.

![Gameplay with the player, balls, and objective HUD](docs/images/current-gameplay.png)

*A November 7, 2026 play session shows the player, airborne balls, the current level, and both objective counters. The debug HUD shows the restored aggregate total.*

## Start fresh or pick up your count

The startup and pause menus show your last saved bounce total. On the startup menu, **Esc** exits. When the slot is empty, the menu shows **No saved game.** and disables **Continue**. Choose **Delete Save** on the startup menu to clear the slot; **New Game** also clears it before starting.

![Startup menu with a saved game](docs/images/current-startup.png)

*The November 7, 2026 play session shows the saved total and the level offered by Continue, alongside New Game, Delete Save, and Exit.*

During gameplay, press **Esc** to pause. Press it again or choose **Resume Game** to return to the arena. Choose **Exit** from the pause menu to save and quit. **Continue** restores your total the next time you play.

## Rules and progression

- **Win:** make the required number of player-ball contacts in the current session. Level 1 requires 10.
- **Lose:** reach the session's non-player bounce limit. Level 1 allows 50; floor, wall, and ball impacts with a physics impulse count. A player contact counts separately.
- **Ball lifetime:** a ball retires after five non-player bounces. Touching the player retires it immediately and schedules a replacement; a ball that reaches five bounces is not automatically replaced.
- **Read the HUD:** the gameplay HUD shows your level, contacts toward the win target, and non-player bounces toward the loss limit. The debug HUD also shows the saved aggregate bounce total in non-Shipping builds.
- **Advance:** a victory saves the next level. Each level adds one player contact to the win target and subtracts one non-player bounce from the loss limit, down to a minimum loss limit of one. For example, level 2 requires 11 contacts and has a limit of 49 bounces.

The game pauses and saves the outcome on either result screen. After victory, **Continue** returns to the initial menu, where **Continue on level N** starts the newly unlocked level. After defeat, **Retry** starts another session at the same level. **New Game** deletes the existing save and resets the level and history; **Delete Save** clears the slot without starting a game. The save keeps the aggregate bounce count and a chronological history of sessions and results.

## Development

### Build and run

1. Open `test_UE_584_G1_c.uproject` in Unreal Engine 5.8 and build the editor target.
2. Open the Third Person map and press **Play**.
3. In a non-Shipping build, enter `Feature.NewGame` in the console at the startup menu to start immediately. In Play In Editor, use the Output Log command field. [Console command implementation](Source/test_UE_584_G1_c/FeatureConsoleCommands.cpp)

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

Gameplay creates one `AFeatureLevelBounds` actor with six invisible collision faces. The initial interior spans 120 × 120 × 60 meters around a center 20 meters above the world origin. Edit `Feature Bounds` in the game-mode defaults to change its center and half-size. The faces block physics balls. Balls return to the pool after five non-player bounces or player contact.

`UFeatureGameplaySession` is a world subsystem that organizes `OnGameplaySessionStart`, `OnGameplaySessionResume`, `OnGameplaySessionPaused`, and `OnGameplaySessionEnded`. Its start workflow creates the enclosure before the initial ball volley. The game mode observes lifecycle events to spawn the player and present screens. Other features can access the session through `World.GetSubsystem<UFeatureGameplaySession>()` and subscribe to its lifecycle events.

### Session results

A victory or defeat pauses gameplay, saves the session, and presents the matching fullscreen result. **Continue** returns to the startup menu after victory. **Retry** reloads the arena and starts a new session after defeat.

### Debug HUD and save diagnostics

The non-Shipping debug HUD shows the live bounce count, the save slot and file path, and the I/O history. Its **Save game** button writes a snapshot while keeping the session open. **Save and quit** exits after a successful write.

Shipping builds skip the debug previews and keep gameplay paused until disk I/O finishes.

### Architecture

- **Game flow.** The gameplay session handles starting, pausing, resuming, and ending gameplay. The game mode observes its events and presents startup, gameplay, pause, victory, and defeat screens. The startup title comes from `ProjectName` in `Config/DefaultGame.ini`. A gameplay objective can call `FinishSession` to supply a result. [Gameplay session](Source/TestFeatures/Features/GameWorkflow/FeatureGameplaySession.h)
- **Physics and state.** The C++ spawner launches single balls or batches inside configurable bounds and reuses pooled actors. Five non-player bounces retire a ball; player contact retires it and schedules a replacement. Balls publish bounce events, a state controller updates the shared total, and the debug HUD observes it. The Unreal log records lifecycle events, launches, bounces, and pool returns.
- **Persistence.** The menu reads the save preview asynchronously. Session saves and loads pause gameplay during disk I/O. The debug HUD adds one-second previews before I/O and after the result and reports the slot, file path, and operation history.
- **Session history.** Saves retain a chronological array of gameplay sessions with UTC start time, bounce count, player-ball blocking-hit count, and result. New Game starts a fresh history. Continue restores it and appends a session. Resume keeps the current record.
- **Session limits.** Edit `Feature Session Limits` in the game-mode defaults. The level-one defaults are ten player-ball contacts for victory and fifty session bounces for defeat. Reaching either limit ends gameplay and presents the fullscreen result menu. Gameplay starts with five active balls and a pool of fifteen.
- **Module boundaries.** The `TestFeatures` runtime module houses the C++ gameplay, UI, and persistence code. The project uses standard Unreal assets and has no third-party plugin dependencies. Blueprint supports supplementary logic. AngelScript use remains a project goal.

Within `TestFeatures/Features`, `Gameplay` contains balls, bounds, and bounce state. `GameWorkflow` contains session and persistence code. `UI` contains menus and HUD panels. Plain C++ spawning and storage helpers live in the sibling `Features_NonUE` folder.

- [Gameplay systems and HUD](Source/TestFeatures/Features/README.md)
- [Runtime helpers](Source/TestFeatures/Features_NonUE/README.md)
- [Project objectives and technical requirements](AGENTS.MD)
- [Completed tasks and capture index](CHANGES.md)

### Local Unreal Editor MCP

For explicitly requested editor work, open `test_UE_584_G1_c.uproject` normally. The project enables Epic's built-in MCP and toolset plugins for editor targets and starts the local server automatically. To turn off automatic startup for your local editor, clear **Auto Start Server** under **Edit → Editor Preferences → Model Context Protocol**.

The trusted project's `.codex/config.toml` points Codex at `http://127.0.0.1:8000/mcp`. If the endpoint is unavailable and Unreal Editor is closed, open the project and wait for the editor to finish loading and the local server to respond. If the current chat does not expose the Unreal tools once the server responds, start a new Codex chat in this project with the editor still open.

### Game evolution

- **Before the main menu.** The debug HUD already showed load progress and restored bounce totals. The November 1, 2026 capture shows an offscreen test that restored 96 bounces. [Historical loading HUD](docs/images/in-game-loading-hud.png)
- **First startup menu (Task 18).** New Game and Continue gave players separate ways into the arena. The menu disabled Continue when the slot was empty. The saved-total summary shown above came later. [Historical startup menu](docs/images/task-18-main-menu.jpg)
