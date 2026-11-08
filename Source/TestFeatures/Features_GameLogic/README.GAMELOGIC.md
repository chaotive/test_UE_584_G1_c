# Game logic

This folder contains the game's rule calculations and the C++ code that connects them to sessions and player movement. The focus is game-logic work; AngelScript is the current tool for expressing simple rules that can change without rebuilding C++.

## Responsibility and ownership

- Level-limit formulas have one implementation in [FeatureLevelRules.as](FeatureLevelRules.as); player speed and bonus transitions live in [FeaturePlayerRules.as](FeaturePlayerRules.as).
- The [level host](FeatureLevelRules.cpp), `UFeatureLevelRules`, extends `UWorldSubsystem` and passes integer inputs and results across the boundary.
- The [player bridge](FeaturePlayerRules.cpp), `UFeaturePlayerRules`, extends `UActorComponent`. It owns contact subscriptions, native movement updates, the expiry timer, and transient bonus state. GameMode supplies its configuration; the character only creates the component alongside its movement and cameras.
- Both hosts use [FFeatureScriptRuntime](FeatureScriptRuntime.cpp) for compilation, function lookup, execution, and native resource ownership. Level rules load once per world; player rules load when each spawned character is configured, with an independent context and no per-frame script polling.
- Game-mode configuration and saved progress remain owned by C++. The script receives values rather than retaining a second copy of game state.
- The [gameplay session](../Features/GameWorkflow/FeatureGameplaySession.cpp) uses the returned limits for both outcome checks and the gameplay HUD.

The level example calculates player-contact and non-player bounce thresholds. The player-character example calculates baseline speed, whether a contact creates or stacks a bonus, and the resulting walking speed. C++ stores the returned bonus count and schedules gameplay-time expiry; the script retains no duplicate configuration or state. Timer expiry clears the transient count and reapplies the scripted baseline. The existing boost refresh and stacking behavior is preserved.

## Change a rule

1. Stop PIE.
2. Edit the relevant function in `FeatureLevelRules.as` or `FeaturePlayerRules.as` and save the file.
3. Start PIE again. The new world creates a fresh host, reads the saved script from disk, and compiles the updated rule.

Script-only formula changes reuse the existing C++ game binary, editor, and runtime library. Live hot reload is optional future work; the stop/edit/start workflow already supports quick rule iteration.

Keep the existing entry-point argument and return types when changing a formula. Level rules use integers; player rules use integers, floats, and a boolean. To add a new input or entry point, update the corresponding C++ bridge and caller, then rebuild C++. Keep the rule itself in one place instead of adding a parallel C++ implementation.

## Develop and verify behavior

Use the [development guide](../../../docs/DEVELOPMENT.md) for builds, tests, and diagnostics. The [architecture view](../../../README.md#game-logic-architecture) explains the value-based boundary and its intended benefits. Script compilation diagnostics are logged by the host; changes to the bindings need C++ compilation as well.

All authored `.as` source and project-specific C++ hosts and bindings stay together here. Packaged games use a staged script copy, so distributing a rule update also requires updating that deployed content.

## Runtime dependency

The host uses a prebuilt AngelScript library. Its [dependency guide](../../ThirdParty/FeatureAngelScriptRuntime/README.ANGELSCRIPT.md) covers the pinned version, source cache, library rebuilds, and upgrades. Those operations are separate from ordinary game-rule edits.
