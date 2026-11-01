# Plain C++ feature helpers

This folder holds classes that are not Unreal-reflected `UCLASS` types and do not derive from `UObject`. They still use Unreal APIs and types.

- [FTrainingBallSpawner](TrainingBallSpawner.h) creates and pools the world-owned balls defined in [TrainingBall.h](../Features/TrainingBall.h).
- [FTrainingDebugHUDSpawner](TrainingDebugHUDSpawner.h) creates the native debug widget defined in [TrainingDebugHUD.h](../Features/TrainingDebugHUD.h) and adds it to a local player's screen.

Include these helpers through `Features_NonUE/TrainingBallSpawner.h` or `Features_NonUE/TrainingDebugHUDSpawner.h` from a consumer with a `TrainingFeatures` module dependency.

The [feature guide](../Features/README.md) explains startup, spawning, pooling, event subscriptions, UI creation, ownership, and performance tradeoffs. This folder split changes code organization without changing those flows or their runtime costs.

`FTrainingSaveGameStorage` loads and saves scalar bounce-total snapshots using `Features/TrainingSaveGame.h`. It exposes the authoritative local slot name, desktop file path, and load result for diagnostics. It has no dependency on the live game state or HUD; the main feature guide documents the save slot, lifecycle, failure behavior, and asynchronous disk operations and the synchronous forced-teardown fallback.

Task 17 moves these helpers into the TrainingFeatures runtime module alongside the UE classes. TRAININGFEATURES_API exports their symbols. NonUE means non-UObject, not independent of Unreal Engine. The extraction changes build organization without adding runtime work.


Storage reads bytes on a worker thread and constructs/deserializes the known UTrainingSaveGame on the game thread. Using the slot's known type preserves the payload across the task 17 class-package move; it avoids the stock loader's lookup of the historical class name. Both async and synchronous reads use this path; async saving remains Unreal's implementation. No actor or widget is accessed by the read worker.

Review follow-up: the known-type save reader now returns deserialization success separately from the bounce count. Empty data and archive read errors produce the existing load-failed/zero result rather than a successful-load message. This reader assumes this slot contains our save payload; it is not a general format validator or a schema migration system.
