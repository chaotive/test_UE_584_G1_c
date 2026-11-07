# Feature Sandbox changes

This index follows the task numbers in [AGENTS.MD](AGENTS.MD). The November 7, 2026 captures come from Unreal Engine play sessions and can illustrate several related tasks. The source links document implementation details that a still image cannot establish.

## Completed tasks

| Task | Change | Capture and evidence |
| --- | --- | --- |
| 1 | Spawn a ball when gameplay starts | [Arena and debug HUD](docs/images/feature-debug-hud.png) |
| 2 | Log game-mode lifecycle events | [Arena after startup](docs/images/feature-debug-hud.png) · [Lifecycle hooks](Source/test_UE_584_G1_c/test_UE_584_G1_cGameMode.cpp) |
| 3 | Randomize the launch trajectory | [Launch velocity in the Output Log](docs/images/feature-output-log.png) · [Launch logic](Source/TestFeatures/Features/Gameplay/FeatureBall.cpp) |
| 4 | Give each ball its own actor class | [Arena and debug HUD](docs/images/feature-debug-hud.png) · [Ball actor](Source/TestFeatures/Features/Gameplay/FeatureBall.cpp) |
| 5 | Launch several balls within configurable bounds | [Multiple airborne balls after Continue](docs/images/feature-continue.png) · [Batch spawner](Source/TestFeatures/Features_NonUE/FeatureBallSpawner.cpp) |
| 6 | Support single and batch spawning with indexed names | [Indexed ball names in the Output Log](docs/images/feature-output-log.png) · [Spawner and pool indices](Source/TestFeatures/Features_NonUE/FeatureBallSpawner.cpp) |
| 7 | Remove a ball from play after its second bounce | [Arena and debug HUD](docs/images/feature-debug-hud.png) · [Bounce handling](Source/TestFeatures/Features/Gameplay/FeatureBall.cpp) |
| 8 | Reuse inactive balls | [Pool growth in the Output Log](docs/images/feature-output-log.png) · [Pool implementation](Source/TestFeatures/Features_NonUE/FeatureBallSpawner.cpp) |
| 9 | Cache the ball mesh component | [Arena and debug HUD](docs/images/feature-debug-hud.png) · [Mesh reference](Source/TestFeatures/Features/Gameplay/FeatureBall.h) |
| 10 | Track the shared bounce total in game state | [Game-state total increasing in the Output Log](docs/images/feature-output-log.png) · [Game state](Source/TestFeatures/Features/Gameplay/FeatureGameState.cpp) |
| 11 | Forward ball events through a game-state controller | [Controller receiving bounce events](docs/images/feature-output-log.png) · [Event bridge](Source/TestFeatures/Features/Gameplay/FeatureGameStateController.cpp) |
| 12 | Create the debug HUD in C++ | [Debug HUD](docs/images/feature-debug-hud.png) · [Widget layout](Source/TestFeatures/Features/UI/FeatureDebugHUD.cpp) |
| 13 | Display live bounces in the HUD | [Zero at launch](docs/images/feature-debug-hud.png) · [Ten after the volley](docs/images/feature-save-result.png) |
| 14 | Save and restore the bounce total | [Successful save](docs/images/feature-save-result.png) · [Restored total](docs/images/feature-continue.png) |
| 15 | Pause gameplay during save and load | [Paused phases in the save history](docs/images/feature-save-result.png) · [Pause and quit policy](Source/TestFeatures/Features/GameWorkflow/FeatureSaveGameSubsystem.cpp) |
| 16 | Hold the debug previews for one second before and after I/O | [Preview phases in the load history](docs/images/feature-continue.png) · [Preview timer](Source/TestFeatures/Features/UI/FeatureSaveGamePanel.cpp) |
| 17 | Extract the independent TestFeatures runtime module | [Module log category](docs/images/feature-output-log.png) · [Module definition](Source/TestFeatures/TestFeatures.Build.cs) |
| 18 | Show the configured title and startup actions | [Feature Sandbox startup menu](docs/images/feature-startup.png) |
| 30 | Show the saved total or the empty-slot message | [Empty slot](docs/images/feature-startup.png) · [Saved total](docs/images/feature-startup-saved.png) |
| 31 | Add the non-Shipping `Feature.NewGame` command | [Arena after entering the command in the Output Log](docs/images/feature-debug-hud.png) · [Command registration](Source/test_UE_584_G1_c/FeatureConsoleCommands.cpp) |
| 32 | Show the debug HUD in non-Shipping builds | [Development HUD](docs/images/feature-debug-hud.png) · [Shipping gate in source](Source/test_UE_584_G1_c/test_UE_584_G1_cPlayerController.cpp) |
| 34 | Package Win64 builds in Development, DebugGame, and Shipping configurations | [Packaging instructions](README.md#package-for-win64) · [Cook settings](Config/DefaultGame.ini) |
| 35 | Spawn one configurable collision enclosure when gameplay starts | [Runtime collision capture](docs/images/feature-level-bounds.png) · [Bounds actor](Source/TestFeatures/Features/Gameplay/FeatureLevelBounds.cpp) · [Gameplay session](Source/TestFeatures/Features/GameWorkflow/FeatureGameplaySession.cpp) |
| 39 | Organize feature classes into UI, Gameplay, and GameWorkflow folders | [Gameplay after the move](docs/images/feature-organized-gameplay.png) · [Folder responsibilities](Source/TestFeatures/Features/README.md#folder-organization) |

The gameplay refinement introduces `UFeatureGameplaySession`, a world subsystem with start, resume, pause, and end workflows. The game mode observes its lifecycle events to manage the player and screens. The collision capture verifies the enclosure geometry. Ball retirement still occurs after two bounces.

Verification on November 7, 2026 passed the Unreal 5.8 Development editor build and a standalone lifecycle check through startup, gameplay, pause, resume, and victory. The lifecycle process exited with code zero.

## To do

The remaining tasks are tracked in [the backlog](AGENTS.MD#to-do). Their completion captures will be added as the tasks are implemented.
