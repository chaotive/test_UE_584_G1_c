## SUMMARY

This is an Unreal Engine project that we will use as
sandbox to try different coding patterns and features.

## CODE STANDARDS

1. Use as little files as possible, this is a simple project.
2. Do not add defensive code, it is more important to give clarity to the code features understanding and reading.
3. No need to ask editor or other guards unless required to.
4. Commits: if a task is being implemented, always mention it on the description.
5. Commits: commits should give a brief but useful description of changes.
6. Avoid too many one liners, when possible, separate in functions that act as groupers to have a better top down understanding of code.
7. Above every function declaration and class property, add two separate one-line comments: first describe its functional purpose; second explain the types, pointer/reference semantics, ownership, and why any wrapper (such as TArray or TWeakObjectPtr) is used. For functions without pointers or references, say so in the technical line.

## TASKS

### DONE

1. Create a ball through code, which should be created at game mode start.
2. Please add on BeginPlay() and all main lifecycle events messages to the ue log. Add main other lifecycle functions and just do the log if needed be.
3. Have the training ball start moving on a random direction, following a random trajectory.
4. Make that the TrainingBall becomes its own simple class, so then the spawner actually spawns them as instantiated objects. The gamemode will call the spawner, and the spawner will instantiate and spawn trainingballs as needed.
5. Spawn several random balls using the trainingball, and not just 1 anymore. Adapt the current properties for the initial spawn spot to become a range and not just an specific vector, allowing each ball to spawn within the range.
6. Allow still to have spawner to spawn a single ball with just public spawn, and spawn in multiple with a second public function, that calls spawn several times. The spawned balls remain in a dynamic list and when single spawned, the name follows the list index.
7. Now makes that each ball is destroyed after bouncing 2 times.
8. Implement objects pooling for the balls spawner.

### TO DO

9.
