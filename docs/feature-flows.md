# Feature Sandbox: system flows

These diagrams show the current C++ relationships and runtime flows. Solid arrows show calls, data, or player actions; arrows labelled “event” show notifications between systems. Limits and initial pool sizes are configurable defaults.

## Gameplay: balls, contacts, and objectives

```mermaid
flowchart LR
    Session["Gameplay session<br/>starts a round"] --> Bounds["Level bounds<br/>contain physics balls"]
    Session --> Pool["Ball spawner and pool<br/>15 prewarmed · 5 active"]
    Pool -->|activate| Ball["Ball actor<br/>random position and velocity"]
    Ball -->|"event: non-player bounce"| Bridge["Game-state controller"]
    Ball -->|"event: player contact"| Bridge
    Ball -->|"5 non-player bounces"| Retire["Deactivate and return to pool"]
    Ball -->|"player contact event"| Session
    Session -->|"retire; replacement next tick"| Pool
    Bridge -->|"update counts"| State["Game state<br/>session stats · saved total · level"]
    State -->|"stats event"| Session
    Session --> Limits{"Session limit reached?"}
    Limits -->|"contacts ≥ target"| Win["Victory"]
    Limits -->|"bounces ≥ limit"| Lose["Defeat"]
    State -->|"stats and level events"| HUD["Gameplay and debug HUDs"]
    Retire -. "available for later reuse" .-> Pool
    classDef logic fill:#dbeafe,stroke:#2563eb,color:#172554
    classDef event fill:#dcfce7,stroke:#16a34a,color:#14532d
    classDef result fill:#fef3c7,stroke:#d97706,color:#78350f
    class Session,Pool,Ball,Bounds,Bridge,State logic
    class Retire,HUD event
    class Limits,Win,Lose result
```

The session counts accepted non-player physics hit callbacks; one ball-to-ball collision can generate a callback for each ball. A ball retired after five bounces waits in the pool. Player contact retires a ball and schedules a replacement while the session is active. See [the ball](../Source/TestFeatures/Features/Gameplay/FeatureBall.cpp), [state controller](../Source/TestFeatures/Features/Gameplay/FeatureGameStateController.cpp), and [session](../Source/TestFeatures/Features/GameWorkflow/FeatureGameplaySession.cpp).

## UI: construction, actions, and observation

```mermaid
flowchart LR
    Mode["Game mode<br/>owns flow state"] -->|"present or dismiss"| Menu["Main menu archetype<br/>shared layout and action wiring"]
    Views["Menu view definitions<br/>initial · pause · victory · defeat"] -->|"heading and ordered actions"| Menu
    Menu -->|"action event"| Mode
    Menu -->|"load preview / delete request"| Save["Save subsystem and storage"]
    Save -->|"deletion event / preview"| Menu
    Controller["Player controller"] -->|"create and remove"| GameHUD["Gameplay HUD"]
    Controller -->|"non-Shipping: create and remove"| DebugHUD["Debug HUD"]
    DebugHUD --> BallPanel["Ball panel"]
    DebugHUD --> SavePanel["Save panel"]
    State["Game state"] -->|"stats and level events"| GameHUD
    State -->|"total and level events"| BallPanel
    Save -->|"status event"| SavePanel
    SavePanel -->|"save requests and preview timing"| Save
    Session["Gameplay session"] -->|"configured limits"| GameHUD
    classDef ui fill:#f3e8ff,stroke:#9333ea,color:#3b0764
    classDef systems fill:#dbeafe,stroke:#2563eb,color:#172554
    class Menu,Views,GameHUD,DebugHUD,BallPanel,SavePanel ui
    class Mode,Controller,State,Save,Session systems
```

The widgets own their subscriptions and initial synchronization. The player controller manages HUD lifetime; it does not connect state events to labels. The save panel drives the non-Shipping preview timers. See [menu views](../Source/TestFeatures/Features/UI/Views/FeatureMenuViews.cpp), [menu](../Source/TestFeatures/Features/UI/Archetypes/FeatureMainMenu.cpp), and [HUD](../Source/TestFeatures/Features/UI/Archetypes/FeatureGameplayHUD.cpp).

## Game workflow: start, pause, save, and results

```mermaid
flowchart TD
    Start(["Startup menu"]) -->|"New Game: delete slot"| New["Start level 1 with fresh progress"]
    Start -->|"Continue on level N"| Load["Load saved progress"]
    Start -->|"Delete Save"| Start
    Start -->|"Exit / Escape"| Exit(["Exit"])
    New --> Play["Playing<br/>spawn player and ball volley"]
    Load -->|"load completes"| Play
    Play -->|"Escape"| Pause["Pause menu"]
    Pause -->|"Resume / Escape"| Play
    Pause -->|"Exit: save and quit"| Exit
    Play -->|"win limit"| Victory["Victory<br/>advance saved level"]
    Play -->|"loss limit"| Defeat["Defeat<br/>retain saved level"]
    Victory --> SaveResult["Record result and request save"]
    Defeat --> SaveResult
    SaveResult -->|"Victory: Continue"| Start
    SaveResult -->|"Defeat: Retry"| Load
    SaveResult -->|"Exit: save and quit"| Exit
    classDef menu fill:#f3e8ff,stroke:#9333ea,color:#3b0764
    classDef work fill:#dbeafe,stroke:#2563eb,color:#172554
    classDef outcome fill:#fef3c7,stroke:#d97706,color:#78350f
    class Start,Pause menu
    class New,Load,Play,SaveResult work
    class Victory,Defeat,Exit outcome
```

Persistence gates menu actions while I/O is busy and restores the prior pause state afterwards. A successful victory increments the saved level; defeat and Retry keep it. See [game mode](../Source/test_UE_584_G1_c/test_UE_584_G1_cGameMode.cpp), [session](../Source/TestFeatures/Features/GameWorkflow/FeatureGameplaySession.cpp), and [persistence subsystem](../Source/TestFeatures/Features/GameWorkflow/FeatureSaveGameSubsystem.cpp).
