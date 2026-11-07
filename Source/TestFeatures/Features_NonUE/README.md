# Plain C++ feature helpers

This folder holds classes that are not Unreal-reflected `UCLASS` types and do not derive from `UObject`. They still use Unreal APIs and types.

- [FFeatureBallSpawner](FeatureBallSpawner.h) creates and pools the world-owned balls defined in [FeatureBall.h](../Features/Gameplay/FeatureBall.h). The gameplay session retains this helper and requests the initial volley.
- [FFeatureDebugHUDSpawner](FeatureDebugHUDSpawner.h) creates the native debug widget defined in [FeatureDebugHUD.h](../Features/UI/FeatureDebugHUD.h) and adds it to a local player's screen. The player controller calls it only in non-Shipping builds.

Include these helpers through `Features_NonUE/FeatureBallSpawner.h` or `Features_NonUE/FeatureDebugHUDSpawner.h` from a consumer with a `TestFeatures` module dependency.

The ball spawner assigns indexed actor names in every build. It also sets the matching World Outliner label in editor builds.

The [feature guide](../Features/README.md) explains startup, spawning, pooling, event subscriptions, UI creation, ownership, and performance tradeoffs. This folder split changes code organization without changing those flows or their runtime costs.

`FFeatureSaveGameStorage` loads and saves progress snapshots using `Features/GameWorkflow/FeatureSaveGame.h`. A snapshot includes the aggregate bounce total and each session's start time, bounces, and player-ball contacts. Storage provides the local slot name, desktop file path, and load result for diagnostics.

Task 17 moved these helpers into the runtime module alongside the Unreal-reflected classes. The module is now named `TestFeatures` and exports the helpers with `TESTFEATURES_API`.

Storage uses Unreal's asynchronous loading and saving APIs. An asynchronous load returns an optional progress snapshot so the menu can distinguish a valid zero total from a missing or unreadable save. Gameplay starts with empty progress when loading returns no usable snapshot.

Compatibility policy: old data may be discarded. The configured game mode and player controller still create the current feature classes in code, so a missing save does not prevent startup.
