# Game logic

This folder contains the game's rule calculations and the C++ code that connects them to sessions, balls, and player movement. The focus is game-logic work; AngelScript is the current tool for expressing simple rules that can change without rebuilding C++.

## Responsibility and ownership

- Level limits, session outcomes, and level progression have one implementation in [FeatureLevelRules.as](FeatureLevelRules.as); player speed and bonus transitions live in [FeaturePlayerRules.as](FeaturePlayerRules.as). [FeatureBallRules.as](FeatureBallRules.as) owns bounce classification, retirement, and replacement eligibility.
- The [level host](FeatureLevelRules.cpp), `UFeatureLevelRules`, extends `UWorldSubsystem` and passes integer inputs and the registered session-result enum across the boundary.
- The [player bridge](FeaturePlayerRules.cpp), `UFeaturePlayerRules`, extends `UActorComponent`. It observes player contacts from the ball manager and owns native movement updates, the expiry timer, and transient bonus state. GameMode supplies its configuration; the character only creates the component alongside its movement and cameras.
- The [ball bridge](FeatureBallRules.cpp), `UFeatureBallRules`, extends `UWorldSubsystem`. It translates native collision facts into values and returns scripted action flags. The ball applies counting and retirement effects. The manager calls the same bridge for replacement eligibility and owns the native queue and pool. [Ball manager](../Features/Gameplay/FeatureBallManager.h)
- All three bridges use [FFeatureScriptRuntime](FeatureScriptRuntime.cpp) for compilation, function lookup, execution, and native resource ownership. Level and ball rules each load once per world; player rules load when each spawned character is configured. Each bridge owns an independent runtime and reusable context, with no per-frame script polling.
- Game-mode configuration and saved progress remain owned by C++. The script receives values rather than retaining a second copy of game state.
- The [gameplay session](../Features/GameWorkflow/FeatureGameplaySession.cpp) uses the returned limits for both outcome checks and the gameplay HUD.

The level example calculates thresholds, selects InProgress/Victory/Defeat, and returns the next saved level. Victory takes precedence if both limits are reached and advances the level; other results retain it. C++ applies the returned result, pauses play, stores progress, and saves. Native result/action enums are registered before compilation; scripts do not maintain separate numeric definitions.

The ball example ignores non-player contacts without solver impulse, counts accepted non-player bounces, and retires balls at their rebound limit. Player contacts retire without counting a non-player bounce. Retired balls are replaced only during active gameplay. C++ translates collision data, publishes events, controls physics, and schedules replacements; it does not repeat those decisions.

The player-character example calculates baseline speed, whether a contact creates or stacks a bonus, and the resulting walking speed. C++ stores the returned bonus count and schedules gameplay-time expiry; the script retains no duplicate configuration or state. Timer expiry clears the transient count and reapplies the scripted baseline. The existing boost refresh and stacking behavior is preserved.

## Change a rule

1. Stop PIE.
2. Edit the relevant function in `FeatureLevelRules.as`, `FeaturePlayerRules.as`, or `FeatureBallRules.as` and save the file.
3. Start PIE again. The new world creates fresh level/ball hosts; starting gameplay configures the player bridge. Each reads and compiles its saved script.

Script-only rule changes reuse the existing C++ game binary, editor, and runtime library. Live hot reload is optional future work; the stop/edit/start workflow already supports quick rule iteration.

Keep existing entry-point argument and return types when changing a rule. The bridges pass integers, floats, booleans, and registered native enums. To add a new input or entry point, update the corresponding C++ bridge and caller, then rebuild C++. Keep the rule itself in one place instead of adding a parallel C++ implementation. When future behavior changes a property during gameplay, consider placing its decision or calculation here, as required by the [project's design rule](../../../AGENTS.MD#code-standards).

## Develop and verify behavior

Use the [standalone unit tests](../../../docs/DEVELOPMENT.md#standalone-unit-tests) to check changes to ball, level, and player rules without building or launching Unreal. Each spec in `Tests/` includes its production rule file. The [architecture view](../../../README.md#game-logic-architecture) explains the value-based boundary and its intended benefits. Script compilation diagnostics are logged by the host; changes to the bindings need C++ compilation as well.

All authored `.as` source and project-specific C++ hosts and bindings stay together here. Keep runtime scripts at the folder root so packaged games stage them. The `Tests/` subfolder is for development only. Distributing a rule update also requires updating the staged copy.

## Runtime dependency

The bridges use a prebuilt AngelScript library. Its [dependency guide](../../ThirdParty/FeatureAngelScriptRuntime/README.ANGELSCRIPT.md) covers the pinned version, source cache, library rebuilds, and upgrades. Those operations are separate from ordinary game-rule edits.
