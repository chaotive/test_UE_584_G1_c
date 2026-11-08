# Game logic

This folder contains the game's rule calculations and the C++ code that connects them to the session. The focus is game-logic work; AngelScript is the current tool for expressing simple rules that can change without rebuilding C++.

## Responsibility and ownership

- Rule formulas have one implementation in [FeatureLevelRules.as](FeatureLevelRules.as).
- The [C++ host](FeatureLevelRules.cpp), `UFeatureLevelRules`, extends `UWorldSubsystem`. It loads the script and passes integer inputs and results across the boundary.
- Game-mode configuration and saved progress remain owned by C++. The script receives values rather than retaining a second copy of game state.
- The [gameplay session](../Features/GameWorkflow/FeatureGameplaySession.cpp) uses the returned limits for both outcome checks and the gameplay HUD.

The current example calculates the player-contact target and non-player bounce limit from the level and configured settings. Extend the rule interface when another concrete game-logic use case needs it.

## Change a rule

1. Stop PIE.
2. Edit the relevant function in `FeatureLevelRules.as` and save the file.
3. Start PIE again. The new world creates a fresh host, reads the saved script from disk, and compiles the updated rule.

Script-only formula changes reuse the existing C++ game binary, editor, and runtime library. Live hot reload is optional future work; the stop/edit/start workflow already supports quick rule iteration.

Keep the existing integer inputs and return types when changing a formula. To add a new input or rule entry point, update the [host declaration](FeatureLevelRules.h), its binding code, and the session caller, then rebuild C++. Keep the rule itself in one place instead of adding a parallel C++ implementation.

## Develop and verify behavior

Use the [development guide](../../../docs/DEVELOPMENT.md) for builds, tests, and diagnostics. The [architecture view](../../../README.md#game-logic-architecture) explains the value-based boundary and its intended benefits. Script compilation diagnostics are logged by the host; changes to the bindings need C++ compilation as well.

All authored `.as` source and project-specific C++ hosts and bindings stay together here. Packaged games use a staged script copy, so distributing a rule update also requires updating that deployed content.

## Runtime dependency

The host uses a prebuilt AngelScript library. Its [dependency guide](../../ThirdParty/FeatureAngelScriptRuntime/README.ANGELSCRIPT.md) covers the pinned version, source cache, library rebuilds, and upgrades. Those operations are separate from ordinary game-rule edits.
