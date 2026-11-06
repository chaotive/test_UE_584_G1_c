# Plain C++ feature helpers

This folder holds classes that are not Unreal-reflected `UCLASS` types and do not derive from `UObject`. They still use Unreal APIs and types.

- [FFeatureBallSpawner](FeatureBallSpawner.h) creates and pools the world-owned balls defined in [FeatureBall.h](../Features/FeatureBall.h).
- [FFeatureDebugHUDSpawner](FeatureDebugHUDSpawner.h) creates the native debug widget defined in [FeatureDebugHUD.h](../Features/FeatureDebugHUD.h) and adds it to a local player's screen.

Include these helpers through `Features_NonUE/FeatureBallSpawner.h` or `Features_NonUE/FeatureDebugHUDSpawner.h` from a consumer with a `TestFeatures` module dependency.

The [feature guide](../Features/README.md) explains startup, spawning, pooling, event subscriptions, UI creation, ownership, and performance tradeoffs. This folder split changes code organization without changing those flows or their runtime costs.

`FFeatureSaveGameStorage` loads and saves bounce totals using `Features/FeatureSaveGame.h`. It provides the local slot name, desktop file path, and load result for diagnostics. The [feature guide](../Features/README.md) covers asynchronous disk operations and the synchronous fallback during forced teardown.

Task 17 moved these helpers into the runtime module alongside the Unreal-reflected classes. The module is now named `TestFeatures` and exports the helpers with `TESTFEATURES_API`.

Storage uses standard Unreal asynchronous loading and saving; object creation and deserialization follow the engine API. Missing or incompatible saves start from zero. No raw payload reader or old-class migration remains.

Compatibility policy: old data may be discarded. The configured game mode and player controller still create the current feature classes in code, so a missing save does not prevent startup.
