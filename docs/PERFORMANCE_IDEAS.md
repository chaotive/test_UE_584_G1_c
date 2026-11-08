# Performance and design improvement ideas

Reviewed against project source on November 8, 2026. This is a source review, not a benchmark. These are investigation ideas, not implemented optimizations or approved implementation tasks. See the [development guide](DEVELOPMENT.md) for running and testing the project.

## Current baseline

The session prewarms five actors and activates five balls by default. Retirement schedules replacement through the same pool on the next tick, after either player contact or the configured non-player rebound limit. The old three-times prewarm ratio is historical. Inactive actors are hidden with physics and collision disabled; mesh and visual material assets are shared. Each ball has its own physical-material subobject.

State and HUD updates use events, with subscriptions attached at actor creation rather than on every reuse. Preserve those boundaries: removing delegates or merging UI with gameplay has no demonstrated performance benefit.

Normal persistence already uses asynchronous disk access, pausing gameplay until completion. The old sleep delays, ticker previews, and modal painting handshake are absent from the current subsystem. Progress contains the level, aggregate count, and chronological sessions, rather than one integer. Forced teardown retains a synchronous save fallback.

AngelScript compiles once per world initialization and reuses its context and function handles. The prebuilt library avoids rebuilding SDK sources during ordinary builds; script compilation and interpreter execution still happen at runtime. Starting a new PIE world intentionally picks up saved script edits.

## Suggested order of investigation

| Priority | Opportunity | Source evidence | Measure before changing |
| --- | --- | --- | --- |
| First | Reduce detailed logging during performance comparisons. | Spawn, bounce, bridge, and state paths emit ordinary Log messages. | Frame and event-handler time with logging enabled and suppressed. |
| First | Bound diagnostic history and avoid full-history text rebuilds. | Every phase appends a string; every panel refresh joins all entries. | History length, formatting/layout time, retained text memory. |
| First | Track saved-session growth. | Snapshot copies and menu summary loads include every session. | Save bytes, snapshot/serialization time, menu-load time. |
| When scaling | Replace repeated pool scans with available-entry bookkeeping. | Every acquisition starts at the first pool entry. | Scan length, creation time, active and retained actor counts. |
| When scaling | Investigate physics and rendering separately. | Active balls simulate and can collide with each other; random placement permits overlap. | Physics/contact cost, GPU time, collision density, replacement bursts. |
| If visible in traces | Cache derived limits and reduce redundant HUD formatting. | Evaluation and display each call both script functions. | Script executions per event and time in execution/text layout. |

At five balls, additional queues, caches, and scheduling may add more complexity than they save. No frame-rate target or maximum supported ball count has been established.

## Pool acquisition and retained actors

[The spawner](../Source/TestFeatures/Features_NonUE/FeatureBallSpawner.cpp) scans its creation-order weak-reference list from the beginning for every acquisition. Activating N prewarmed balls visits roughly 1 + 2 + ... + N entries: O(N²) acquisition work, excluding construction. Each replacement can scan O(P) entries for a pool of P actors.

If measured costs justify it, maintain a free-entry stack or queue alongside the creation-order list, populated through retirement observation. Preserve stable names and creation-time subscriptions. The extra collection must accurately represent availability; reserving array capacity alone does not remove scans.

The pool has no trimming policy. Inactive world actors remain allocated, and externally destroyed actors can leave unused weak-reference slots. Normal fixed-size replacement should reuse the initial actors, so growth is a future workload concern rather than an established leak. Track peak active and retained counts before introducing caps or trimming. A shared physical material may reduce per-ball allocations if settings remain identical, but measure whether that memory matters.

## Physics, rendering, and logging

[Ball activation and hit handling](../Source/TestFeatures/Features/Gameplay/FeatureBall.cpp) enable simulation and publish contacts synchronously. AlwaysSpawn and independently randomized positions permit initial overlap. At larger counts, contact density can grow much faster than actor count. Spacing and collision filtering are options only if they preserve intended gameplay: ball-to-ball impacts currently contribute to the non-player count.

The counter accepts nonzero-impulse hit callbacks, rather than separately detecting a complete visual rebound. The default retirement limit is five. A collision cooldown or discarded impacts would change rules, not merely optimize them. Physical-material application now occurs in BeginPlay; the old constructor-application concern is no longer a current recommendation.

Several ordinary-verbosity messages accompany each bounce through the ball, [event bridge](../Source/TestFeatures/Features/Gameplay/FeatureGameStateController.cpp), and [state](../Source/TestFeatures/Features/Gameplay/FeatureGameState.cpp). If profiling shows a cost, move repetitive detail to configurable verbosity while retaining lifecycle summaries. Build-time and category filtering affect which log work executes.

Physics bodies, shadows, materials, and the scene itself may dominate CPU or GPU time. Measure those separately from pool lookup and event dispatch.

## Persistence and diagnostic history

[The persistence subsystem](../Source/TestFeatures/Features/GameWorkflow/FeatureSaveGameSubsystem.cpp) retains every status line for its game-instance lifetime. [The save panel](../Source/TestFeatures/Features/UI/Widgets/FeatureSaveGamePanel.cpp) joins the entire array and recomputes the path whenever status changes. For H similar-length entries, each refresh is O(H); rebuilding after every append accumulates O(H²) text work. A scroll box does not virtualize a single large text block.

Consider keeping the current phase/result separate from a bounded recent-history buffer, with full details in logs. Cache stable slot/path presentation only if useful. This also keeps diagnostics readable.

[Saved progress](../Source/TestFeatures/Features/GameWorkflow/FeatureSaveGame.h) contains a chronological session array without a retention limit. [Storage](../Source/TestFeatures/Features_NonUE/FeatureSaveGameStorage.cpp) copies progress into a save object; loading and state initialization also copy snapshots. Asynchronous disk access does not make all object creation, snapshot copying, or serialization asynchronous. [The startup menu](../Source/TestFeatures/Features/UI/Archetypes/FeatureMainMenu.cpp) loads full progress to show level and aggregate total.

Measure save size and game-thread preparation with increasing session counts. A retention/archive policy or summary-oriented layout would change persistence behavior and needs a deliberate decision. Avoid an independently maintained second source of truth for totals.

[State teardown](../Source/TestFeatures/Features/Gameplay/FeatureGameState.cpp) saves synchronously unless a quit save already completed. A result can therefore save asynchronously and again on later teardown. Measure duration and frequency before considering dirty-state tracking, preserving the intended teardown behavior.

## Script calls and HUD updates

[Session evaluation](../Source/TestFeatures/Features/GameWorkflow/FeatureGameplaySession.cpp) calls two script-backed getters on a stats event. [The gameplay HUD](../Source/TestFeatures/Features/UI/Archetypes/FeatureGameplayHUD.cpp) calls both again for display, normally totaling four script executions per active-play stats update. Other callers and transitions can differ. Context and function handles are already reused.

The formulas are small; interpreter entry may cost more than their arithmetic, but that remains unmeasured. If warranted, compute derived limits when level/configuration changes and share results between evaluation and presentation. AngelScript must remain the single implementation of the formulas. A C++ result cache must refresh on all relevant changes, including victory level advancement.

Both objective lines are formatted on every stats update even when only one counter changes. If traces show text/layout cost, update changed fields or coalesce presentation refreshes while recording every gameplay event and evaluating results immediately. Preserve HUD-owned subscriptions and initial synchronization.

## Measurement plan

For a future authorized profiling session, compare the same Win64 build, map, resolution, and logging settings. Record frame-time distributions plus game-thread, physics, Slate, and GPU contributions. Vary ball count, contact density, diagnostic operations, and saved-session count separately; include startup, steady play, replacement bursts, and save/teardown.

No performance gains are claimed by this review. The November 8 Gauntlet follow-up passed all seven smoke checks after test-fixture corrections; that validates selected behaviors, not performance. Editor startup diagnostics remain tracked separately. See the [smoke-check results](DEVELOPMENT.md#smoke-checks) and [tracked bug](../AGENTS.MD#bugs).
