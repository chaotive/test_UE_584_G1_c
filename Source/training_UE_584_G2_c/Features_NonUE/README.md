# Plain C++ feature helpers

This folder holds classes that are not Unreal-reflected `UCLASS` types and do not derive from `UObject`. They still use Unreal APIs and types.

- [FTrainingBallSpawner](TrainingBallSpawner.h) creates and pools the world-owned balls defined in [TrainingBall.h](../Features/TrainingBall.h).
- [FTrainingDebugHUDSpawner](TrainingDebugHUDSpawner.h) creates the native debug widget defined in [TrainingDebugHUD.h](../Features/TrainingDebugHUD.h) and adds it to a local player's screen.

Include these helpers through `Features_NonUE/TrainingBallSpawner.h` or `Features_NonUE/TrainingDebugHUDSpawner.h` from the project module.

The [feature guide](../Features/README.md) explains startup, spawning, pooling, event subscriptions, UI creation, ownership, and performance tradeoffs. This folder split changes code organization without changing those flows or their runtime costs.

`FTrainingSaveGameStorage` loads and saves scalar bounce-total snapshots using `Features/TrainingSaveGame.h`. It has no dependency on the live game state or HUD; the main feature guide documents the save slot, lifecycle, failure behavior, and synchronous I/O costs.
