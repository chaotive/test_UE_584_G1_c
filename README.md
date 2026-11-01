# Unreal Engine training sandbox

A small Unreal Engine 5.8 C++ project for learning actor lifecycles, pooling, event-driven state, native UI, and save/load behavior.

## In-game debug HUD

![The in-game debug HUD showing the loading preview while the world is paused](docs/images/in-game-loading-hud.png)

Runtime capture from the in-game loading verification. The HUD shows the pause phase, local save slot, and desktop file path while the level remains visible. The zero total is the initial value during loading; the saved total is applied before gameplay resumes. Captured after extracting both UTrainingBallPanel and UTrainingSaveGamePanel: the child widgets render the bounce label, controls, and diagnostics inside the same HUD. The engine's temporary shader-preparation message and sky-material warnings are also visible in this startup frame.

## Current flow

- Spawn a configurable batch of balls at random positions and reuse them after two impacts.
- Publish bounce events through a game-state controller into a shared total.
- Show counts and persistence diagnostics in a code-built debug HUD.
- Pause gameplay during asynchronous load/save; keep the HUD rendering and interactive.
- Show one-second real-time previews before I/O and after the result.
- Use **Save game** or **Save and quit** in the HUD. Normal game-window close requests the saved quit flow. Editor Stop and forced teardown have a synchronous save fallback rather than an interactive preview.

`UTrainingDebugHUD` composes the overall overlay and manages shared layout/input. `UTrainingBallPanel` owns bounce labels, initial synchronization, and subscriptions to game-state changes. `UTrainingSaveGamePanel` owns save/load controls, status subscriptions, preview timing, and orderly-close UI handling as a child section of that same HUD. It is not a separate window.

## Read the code

- [Feature guide and lifecycle flows](Source/TrainingFeatures/Features/README.md)
- [Plain C++ helpers](Source/TrainingFeatures/Features_NonUE/README.md)
- [Performance review ideas](PERFORMANCE_IDEAS.md) — a historical source review with current-flow notes
- [Project rules and task list](AGENTS.MD)

Open `training_UE_584_G2_c.uproject` with UE 5.8, build the editor target, and play the Third Person map. Save data uses the local `TrainingSandbox` slot, user index 0. On the default desktop backend, its file is `Saved/SaveGames/TrainingSandbox.sav`.

Code checkups also remove unneeded includes from our own authored code, while retaining direct dependencies and leaving engine, generated, third-party, and unrelated template code alone. See the full scope in AGENTS.MD.

## Documentation images

Preserve useful implementation/test captures under `docs/images` and reference them from this README or the relevant guide. Captions should identify the behavior actually verified and whether a capture predates later changes. Avoid committing generated binaries, logs, or save-game data alongside images.

## Task 17: independent feature module

All training UE classes and non-UObject helpers compile in the `TrainingFeatures` runtime module under `Source/TrainingFeatures`. A runtime module gives this sandbox a separate build and dependency boundary without plugin packaging. The project module depends on it; features do not depend on the project module. The project game mode and player controller remain the integration points.

Headers retain `Features/` and `Features_NonUE/` include paths. Classes export their symbols with `TRAININGFEATURES_API`; feature diagnostics use `LogTrainingFeatures`. Class redirects in `Config/DefaultEngine.ini` preserve old reflected asset references. Storage reads the slot payload into its known save type so saves with the old module-qualified class name still load. Save slot and data remain unchanged. Close the editor, regenerate project files, and rebuild after this migration.

This extraction adds no per-frame work or gameplay threads; no runtime performance improvement is claimed. The runtime screenshot above predates the module extraction.

Validation: UE 5.8 editor build passed. A headless smoke test restored the existing 145-bounce save, resumed gameplay, exercised pooled balls and teardown saving. The original save was restored afterward. UI rendering and controls were not rechecked; existing physics-material initialization errors remain.

Review follow-up: the known-type save reader now returns deserialization success separately from the bounce count. Empty data and archive read errors produce the existing load-failed/zero result rather than a successful-load message. This reader assumes this slot contains our save payload; it is not a general format validator or a schema migration system.
