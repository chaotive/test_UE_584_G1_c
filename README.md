# test_UE_584_G1_c

A third-person mini-game prototype for PC, played with mouse and keyboard. The target is a complete, short play session. Players start from a main menu, enter gameplay with a clear objective and an in-game HUD, and reach a defined end state.

## Play the current build

1. Open `test_UE_584_G1_c.uproject` in Unreal Engine 5.8 and build the editor target.
2. Open the Third Person map and press **Play**.
3. Use **WASD** to move, the **mouse** to look, and **Space** to jump.

The current build opens directly into the level. Balls launch from randomized positions and travel along varied trajectories. The HUD shows the running bounce total, which is saved locally and restored when you return. Use **Save game** to save without leaving or **Save and quit** to end the session.

The module and its reflected save class were renamed. Saves written with the old class path may start the bounce counter at zero on the next launch.

![The Third Person level and in-game HUD during loading](docs/images/in-game-loading-hud.png)

*Earlier prototype capture showing the level and bounce counter while gameplay is paused during loading.*

## Planned milestones

- **Main menu** — Give players a clear way to start a session.
- **Gameplay objective** — Define what players need to accomplish and show their progress in the HUD.
- **End state** — Recognize when the objective has been resolved and show the outcome to the player.

## Development

Core systems use C++ in Unreal Engine 5.8. The `TestFeatures` runtime module contains the training systems and debug HUD. Blueprint supports supplementary logic, and a small AngelScript feature is planned. The project uses standard Unreal assets and has no third-party plugin dependencies.

- [Gameplay systems and HUD](Source/TestFeatures/Features/README.md)
- [Runtime helpers](Source/TestFeatures/Features_NonUE/README.md)
- [Project objectives and technical requirements](AGENTS.MD)
