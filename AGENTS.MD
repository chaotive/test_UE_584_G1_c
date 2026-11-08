# test_UE_584_G1_c - Test Unreal Engine 5.8.4 Game 1 (C-based)

## Main objective

Develop a small playable game prototype in Unreal Engine 5.8. You may use any AI assistance of your choice.

### What we want to see

A working mini-game with a third-person or first-person character that the player controls with a mouse and keyboard. The game should have a simple but complete flow:

- A main menu
- Gameplay
- An end state

Include at least a basic in-game HUD.

The concept, theme, and scope are entirely up to you. Pick something that lets you show how you think about game architecture and what kinds of engineering decisions you make.

### Technical requirements

- Use Unreal Engine 5.8. Please avoid older 5.x versions.
- C++ is required for core systems. Blueprint and AngelScript are allowed for supplementary logic.
- No third-party plugins.
- Standard assets such as Starter Content, free Fab or Marketplace assets, Megascans, and Mixamo are fine. Avoid spending time on art.
- Visual fidelity is not evaluated.

## Specific goals

These goals will guide future tasks:

1. Create a working mini-game with a third-person character that the player controls with a mouse and keyboard.
2. Include a simple but complete flow with a 
   1. main menu, 
   2. gameplay, 
   3. and an end state.
3. Include at least a basic in-game HUD.
4. Show some use of AngelScript.

## Code standards


1. Prioritize cohesive responsibilities, localized dependencies, and decoupling over minimizing file count. Keep the number of files small where those goals allow.
2. Do not add defensive checks or guards unless the task explicitly requests them. Keep feature logic straightforward and readable.
3. Once an action is authorized, proceed without asking for the same confirmation again. Preserve the explicit editor-operation and runtime-validation authorization requirements in rules 17 and 18.
4. Commits: if a task is being implemented, always mention it on the description.
5. Commits: commits should give a brief but useful description of changes.
6. Avoid too many one liners, when possible, separate in functions that act as groupers to have a better top down understanding of code.
7. Above every function declaration and class property, add two separate one-line comments: first describe its functional purpose; second explain the types, pointer/reference semantics, ownership, and why any wrapper (such as TArray or TWeakObjectPtr) is used. For functions without pointers or references, say so in the technical line.
8. Keep README updates limited to the concise corrections needed to match the implementation. Perform deeper documentation reviews, expanded explanations, and documentation rewrites only when the user explicitly requests them.
9. Code separate the non UE classes from the UE classes. The non UE classes should go in Features_NonUE folder, along Features for UE classes.
10. Design for maximum practical decoupling: minimize direct dependencies and prevent dependency knowledge from spreading into unrelated classes. Keep each relationship within the components that need it or a focused binding class; prefer events and narrow interfaces where they preserve clarity without unnecessary abstraction. For example, HUD-to-game-state subscriptions, initial synchronization, and unsubscription belong inside the HUD or a dedicated UI binding class, while the player controller only creates, retains, and removes the HUD and does not know which state events update which widget methods.
11. For every system/class/feature, attempt to use an UE base class definition and extend from it where possible.
12. Keep UI construction, layout, styling, window management, and rendering synchronization in UI classes or the HUD. Handling systems publish state and operation events; they must not build or render UI. Route diagnostic information, including persistence slot, physical file path, phases, and results, to the debug HUD through observation rather than controller-side UI wiring.
13. Take or refresh screenshots only when the user explicitly requests them. Preserve requested runtime captures under `docs/images` and use the latest verified captures in the relevant documentation. Use older screenshots only in a clearly separate game evolution section when they show a meaningful change. Caption each capture with the behavior it verifies and label historical captures with their milestone or date. Keep mockups and unverified states out of runtime evidence.
14. During code checkups, review our own project-produced code for unneeded includes and remove them. Limit this cleanup to code we authored or changed; do not clean up Unreal Engine, generated, third-party, or unrelated template code. Keep direct includes required by used types and APIs rather than relying on incidental transitive includes.
15. When reviewing changes, check the affected prose for spelling and grammar. Report any issues found and corrections made.
16. The Main objective and Specific goals sections are user-owned. Never change them on your own. Report any suggested edits to the user.
17. Perform editor operations only when the user explicitly requests them, and use Unreal Editor MCP for those operations. For an authorized editor task, if the MCP endpoint is unavailable and Unreal Editor is closed, launch this project's `.uproject` so the server starts automatically. If the editor is already starting, wait rather than opening another instance. Wait for startup to finish and for the MCP endpoint to respond before operating the editor. If this chat still does not expose the tools after the server is ready, start a new Codex chat in this project. The agent may close an editor instance it started itself when needed for its work, without asking the user.
18. Default implementation validation is compile-only. Run Unreal Editor, standalone gameplay, automation tests, or other runtime validation only when the user explicitly requests that validation. Keep requested validation focused on the relevant behavior.
19. Move a task to Done only after the user explicitly asks to implement that task and its implementation is complete. Leave related tasks in To do even if their behavior is already implemented.
20. Keep technical project/module identifiers separate from public, user-facing names. Use `Feature` as the prefix for feature class names, source files, and console commands. Public branding and display titles come from configuration and do not require that prefix.
21. Preserve or migrate old save games only when the user explicitly requests save compatibility.
22. Update `CHANGES.md` only when the user explicitly requests a changelog update. When requested, link relevant source and any available verified evidence. A changelog request does not authorize taking screenshots or running the editor or game unless the user also requests those actions.
23. Never create a git commit unless the user explicitly asks to commit.
24. Preserve existing formatting outside the requested work. Allow line-break, indentation, and whitespace changes necessary for an authorized implementation or prose correction; avoid unrelated formatting changes unless explicitly requested.
25. When the user says "commit as new task", inspect both Done and To do task indexes, calculate one greater than the highest index across both lists, and prefix that number with the literal digits "100" to distinguish these tasks. Record the completed task and include its prefixed task number in the commit description.
26. In CHANGES.md, display screenshot evidence directly as inline images rather than linking to them.
27. For related UI views, keep shared construction, styling, and behavior in common UI code, and express each view's differences declaratively in code. Use another data format only when it provides a clear benefit.
28. Development runs, tests, documentation updates, and requested screenshot sessions may use, replace, or delete the game's save slot without preserving player progress or asking for permission.
29. In documentation, link to other documents and prefer displaying pictures directly as inline images.
30. Keep all AngelScript (.as) source files exclusively in `Source/TestFeatures/Features_GameLogic`. Packaged script copies are build artifacts; do not author scripts elsewhere.
31. Keep a single source of truth across C++ and AngelScript: each gameplay rule has one authoritative implementation, without mirrored implementations in the other language. C++ owns configuration and persistent state; scripts receive values and return results. Bindings, snapshots, and test fixtures may pass or copy data, but must not duplicate rule implementations or independently maintained gameplay defaults.
32. Keep project-authored AngelScript bridge code, including C++ script hosts and bindings, in `Source/TestFeatures/Features_GameLogic` alongside the `.as` scripts. This also applies to UE-derived bridge classes as a focused exception to the general UE folder rule. Keep the external SDK/library dependency separate under ThirdParty, with SDK implementation source in the ignored dependency cache.
33. Documentation organization: keep `README.md` as the root entry point; `CHANGES.md` and `AGENTS.MD` may also remain at the root. Keep general supporting guides under `docs/`. Specialized per-folder guides may remain under `Source`, but name them `README.<TOPIC>.md` (for example, `README.FEATURES.md`), never plain `README.md`. Keep historical game-evolution captures consolidated in `CHANGES.md`.
34. The root README has three main sections in this order: Game for players and users; Architecture for high-level software design, responsibilities, patterns, and intended benefits; Development for running and testing the game, making changes, and using the project tools for development. Put detailed workflows in `docs/DEVELOPMENT.md` with a grouped topic index, and link those specifics from the relevant discussion in the root README instead of detached lists of links.
35. When new behavior changes a property during gameplay, consider expressing its decision or calculation as an AngelScript rule in `Features_GameLogic`. Keep configuration and authoritative state in C++, pass values across the bridge, and keep a single implementation of each rule. Native code retains engine effects such as movement, physics, timers, spawning, UI, and persistence; this is a design consideration, not a requirement to script every property or engine operation.

## Tasks

Done entries record completed milestones at the time they were implemented. Preserve their history and mark details superseded by later work; use the current implementation and README for current behavior.

### Done

1. Create a ball through code, which should be created at game mode start.
2. Please add on BeginPlay() and all main lifecycle events messages to the ue log. Add main other lifecycle functions and just do the log if needed be.
3. Have the training ball start moving on a random direction, following a random trajectory.
4. Make that the TrainingBall becomes its own simple class, so then the spawner actually spawns them as instantiated objects. The gamemode will call the spawner, and the spawner will instantiate and spawn trainingballs as needed.
5. Spawn several random balls using the trainingball, and not just 1 anymore. Adapt the current properties for the initial spawn spot to become a range and not just an specific vector, allowing each ball to spawn within the range.
6. Allow still to have spawner to spawn a single ball with just public spawn, and spawn in multiple with a second public function, that calls spawn several times. The spawned balls remain in a dynamic list and when single spawned, the name follows the list index.
7. Now makes that each ball is destroyed after bouncing 2 times.
8. Implement objects pooling for the balls spawner.
9. Avoid recalling GetStaticMeshComponent and just do once, while keeping references to it
10. Add a centralized game state on its own independent class and keep tracking of the total bounces made by any and all balls
11. Decouple the dependency of trainingball to the state and make it emit events so another new class can subscribe to them and the state updated. This class could be a gamestate controller or something like that, which would be to handle arbitrary state changes based on emitted events.
12. Display a code dynamic created debug hud on the game UI, implemented in a way that is decoupled and pure code like TrainingBallSpawner
13. Show the total bounces as they updated on the debug hud
14. Implement simple savegame, and save the state into it just before the level or the game mode stops, and load it just before it resumes, making sure the HUD shows the data correctly when loaded back
15. Display a code generated modal loading/saving screen UI state while the I/O for the game save and load is being used. The game should not be playable until the loaded game state is in, and it should not quit until the saved game is completed.
16. Please give the IO loading UI step a 1 second before and after load just to see it working on save and loads.
17. Move all featured UE and non-UE code into its own independent runtime module.
18. Show the startup menu first with the game title from the project configuration and New Game, Continue, and Exit options.
19. Main menu: "New Game" deletes the current save and starts a fresh session.
21. Main menu: Add "Delete Save" above Exit to clear the save slot.
22. Main menu: Disable "Continue" when the save slot is empty.
23. Main menu: "Exit" closes the game.
24. Main menu: Pressing Esc pauses the game and shows the pause menu.
29. Main menu: Pressing Esc on the initial menu exits the game.
30. Show the saved bounce total below the main menu buttons, or display "No saved game." when the slot is empty.
31. Add the non-Shipping console command `Feature.NewGame` to start a new game immediately from the startup menu.
32. Show the debug HUD menu only in non-Shipping builds.
33. Add Gauntlet smoke checks for menu views, session state, ball pooling, and save serialization.
34. Package Win64 builds in Development, DebugGame, and Shipping configurations.
35. Gameplay: Spawn a configurable collision enclosure around the level at gameplay-session start.
36. Gameplay: Retire balls after five non-player bounces. On player contact, retire the ball and spawn a replacement.
37. End state: End a winning session, record Victory, and show a fullscreen result with Continue returning to the initial menu. (Historical menu flow; superseded by Bug 2.)
38. End state: End a losing session, record Defeat, and show the same result flow.
39. Organization: Move feature code into UI, Gameplay, and GameWorkflow subfolders.
40. Gameplay: Persist a chronological array of started sessions with their statistics.
41. Gameplay: Enable player-ball blocking collisions and record their count in the active session.
42. Gameplay: Configure per-session victory and defeat limits and pause when either is reached.
43. Gameplay: Persist the current level, starting at 1. Victory advances it; defeat and Retry retain it. New Game resets it to 1. Show "Continue on level N" in the initial menu. Increase the win limit and decrease the loss limit by one per level, with a minimum loss limit of one bounce. (Historical loss floor; configuration updated by task 53.)
44. Gameplay: Show a HUD with the current player and non-player bounce counts and the respective win and loss limits.
45. Gameplay: Spawn five balls initially and prewarm a pool three times that size. (Historical pool ratio; current prewarm matches the initial ball count.)
46. End: The defeat screen shows only Retry and Exit. Retry restarts the current level.
47. End: Save the game state whenever an end state is reached.
49. UI: Define the initial, pause, victory, and defeat menu views declaratively in UI/Views, and move the menu and HUD archetypes to UI/Archetypes.
50. UI: Move the ball and save-game panels to UI/Widgets.
51. Documentation: Create infographics showing gameplay, UI, and game workflow system flows and connections.
52. When the ball rebounds on the player, it is retired and respawns
53. Gameplay: Add a configurable minimum non-player rebound loss limit, defaulting to 50.
54. Gameplay: Configure the base player speed multiplier and a temporary speed multiplier and duration after ball contact.
1001601. Build: Add a root PowerShell script that discovers Unreal Engine, generates project files, and packages Development, DebugGame, and Shipping for Win64 into a reusable archive ID defaulting to Dev.

### To do

1501. Mass, as a second element (a square)
1600. generalize spawner with objects pooling

## Bugs

### Done

2. When pressing on continue on victory, it takes you to the initial menu, while it should start the next gameplay level

### To do

1. Investigate editor startup diagnostics in Gauntlet. Earlier runs logged repeated `LogAutomationTest: Error: Condition failed` messages, and a November 8 attempt exited with `Engine initialization failed (InitializationFailure, ExitCode=-1)` before creating the editor log. A later November 8 run outside the restricted execution environment completed all seven `Feature.Smoke` checks successfully after correcting the player-state and timer fixtures, with Gauntlet exit code 0. It still logged 14 `Condition failed` messages before the checks. The original initialization failure was not reproduced, and its cause remains unconfirmed; the suite is no longer blocked. See [the recorded results](docs/DEVELOPMENT.md#smoke-checks).
