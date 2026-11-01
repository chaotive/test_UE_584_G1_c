# Unreal Engine training sandbox

A small Unreal Engine 5.8 C++ project for learning actor lifecycles, pooling, event-driven state, native UI, and save/load behavior.

## In-game debug HUD

![The in-game debug HUD showing the loading preview while the world is paused](docs/images/in-game-loading-hud.png)

Runtime capture from the in-game loading verification. The HUD shows the pause phase, local save slot, and desktop file path while the level remains visible. The zero total is the initial value during loading; the saved total is applied before gameplay resumes. Captured after extracting UTrainingSaveGamePanel: the child widget renders the controls and diagnostics inside the same HUD. The engine's temporary shader-preparation message and sky-material warnings are also visible in this startup frame.

## Current flow

- Spawn a configurable batch of balls at random positions and reuse them after two impacts.
- Publish bounce events through a game-state controller into a shared total.
- Show counts and persistence diagnostics in a code-built debug HUD.
- Pause gameplay during asynchronous load/save; keep the HUD rendering and interactive.
- Show one-second real-time previews before I/O and after the result.
- Use **Save game** or **Save and quit** in the HUD. Normal game-window close requests the saved quit flow. Editor Stop and forced teardown have a synchronous save fallback rather than an interactive preview.

`UTrainingDebugHUD` composes the overall overlay. `UTrainingSaveGamePanel` owns save/load controls, status subscriptions, preview timing, and orderly-close UI handling as a child section of that same HUD. It is not a separate window.

## Read the code

- [Feature guide and lifecycle flows](Source/training_UE_584_G2_c/Features/README.md)
- [Plain C++ helpers](Source/training_UE_584_G2_c/Features_NonUE/README.md)
- [Performance review ideas](PERFORMANCE_IDEAS.md) — a historical source review with current-flow notes
- [Project rules and task list](AGENTS.MD)

Open `training_UE_584_G2_c.uproject` with UE 5.8, build the editor target, and play the Third Person map. Save data uses the local `TrainingSandbox` slot, user index 0. On the default desktop backend, its file is `Saved/SaveGames/TrainingSandbox.sav`.

## Documentation images

Preserve useful implementation/test captures under `docs/images` and reference them from this README or the relevant guide. Captions should identify the behavior actually verified and whether a capture predates later changes. Avoid committing generated binaries, logs, or save-game data alongside images.
