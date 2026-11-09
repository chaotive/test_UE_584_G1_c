# Plain C++ feature helpers

The `Source/TestFeatures/Features_NonUE` folder holds classes that are not Unreal-reflected `UCLASS` types and do not derive from `UObject`. They still use Unreal APIs and types.

The console command registrar exposes `Feature.NewGame` in non-Shipping builds and routes it through the menu controller. [FeatureConsoleCommands.cpp](FeatureConsoleCommands.cpp)

Include the storage helper through `Features_NonUE/FeatureSaveGameStorage.h` from a consumer with a `TestFeatures` module dependency.

HUD creation belongs directly to the player controller; layout, styling, and input behavior stay in the HUD. Ball creation and pooling belong to the Unreal-derived [ball manager](../Features/Gameplay/FeatureBallManager.h) in `Gameplay`. It assigns indexed actor names and matching World Outliner labels in editor builds.

The [feature guide](../Features/README.FEATURES.md) explains startup, spawning, pooling, event subscriptions, UI creation, ownership, and performance tradeoffs. This folder split changes code organization without changing those flows or their runtime costs.

`FFeatureSaveGameStorage` loads and saves progress snapshots using `Features/GameWorkflow/FeatureSaveGame.h`. A snapshot includes the aggregate bounce total and each session's start time, bounces, player-ball contacts, and result. Storage provides the local slot name, desktop file path, and load result for diagnostics.

Task 17 moved these helpers into the runtime module alongside the Unreal-reflected classes. The `TestFeatures` module exports the storage helper with `TESTFEATURES_API`. The console command registrar is internal to the module.

Storage uses Unreal's asynchronous loading and saving APIs. An asynchronous load returns an optional progress snapshot so the menu can distinguish a valid zero total from a missing or unreadable save. Gameplay starts with empty progress when loading returns no usable snapshot.

Compatibility policy: old data may be discarded. The configured game mode and player controller still create the current feature classes in code, so a missing save does not prevent startup.
