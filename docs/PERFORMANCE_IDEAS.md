# Performance and design improvement ideas

Historical review: the later in-game HUD refactor replaces the sleep/flush/modal implementation discussed below with paused gameplay, core-ticker previews, and asynchronous I/O. Those observations describe the earlier snapshot, not the current implementation. See the [feature walkthrough](../Source/TestFeatures/Features/README.FEATURES.md) for the current flow.

Task 17 follow-up: feature source links now point into the independent TestFeatures runtime module; this build boundary does not establish a runtime performance gain.

Current follow-up (2026-11-01): asynchronous disk access and nonblocking preview timing are implemented, so those recommendations below are historical rather than outstanding work. A runtime loading smoke test verified rendering and resume ordering after the specialized save-panel extraction. Save/quit interactions and failure/teardown cases still need verification. Preview duration is still fixed, diagnostic history still grows across operations, and serialization/object creation still consume game-thread time. The additional child widget organizes responsibilities; no performance improvement has been measured from that extraction.

Review date: 2026-11-01. Scope: our feature classes, non-UObject helpers, and their game mode/player controller integration, including the pending task 16 preview delays.

**This is a source review, not a benchmark.** No implementation changes, new game runs, save-file writes, or profiling captures were made for this review. Suggested changes below are ideas, not approved implementation tasks. Existing uncommitted task 16 code is left intact.

## Overall assessment

For the default five-ball learning sandbox, the implementation favors readable flow and is reasonably economical in several places. That does not establish a frame-rate target or a supported maximum ball count. Physics, rendering, hardware, editor overhead, and enabled logging must be measured before assigning a numerical performance rating.

The most obvious user-visible delay is intentional: the UI sleeps one second before I/O and another after completion. A normal load followed later by a save adds approximately four seconds of preview waiting in total, excluding disk and rendering time. That is a teaching feature, not evidence that serializing one integer is slow.

### Choices worth keeping

- Gameplay and HUD counts update through events, not a custom per-frame polling loop.
- Each pooled ball is constructed once; reuse resets its state and preserves subscriptions.
- Inactive balls are hidden with simulation and collision disabled.
- The sphere mesh asset is shared; its geometry is not cloned per ball.
- The cached mesh component makes reuse straightforward. Its getter was already cheap, so the cache should not be advertised as a major optimization.
- Storage snapshots contain a scalar, not an actor graph or UI references.
- UI rendering stays in UI classes; the persistence coordinator publishes state and events.

## Suggested order of investigation

| Priority | Idea | Why investigate it? | Evidence level |
| --- | --- | --- | --- |
| 1 | Make preview waiting an explicit optional teaching setting. | Two seconds per operation are deliberately spent sleeping. | Directly visible in code; elapsed user experience not measured here. |
| 1 | Verify initialization/teardown and persistence behavior in actual game runs. | Compilation alone does not demonstrate correct modal presentation, shutdown ordering, or disk recovery. | Runtime verification is still incomplete for the latest work. |
| 2 | Profile with detailed logs enabled and reduced. | Several messages and formatted strings accompany each spawn/bounce. | Enabled work is visible; its relative cost is unmeasured. |
| 2 | Bound diagnostic history and avoid rebuilding an ever-growing string. | History and total formatting work grow with repeated operations. | Algorithmic growth is visible. |
| 3 | Add an available-ball list if pool searches become expensive. | Current requests scan all pool entries, often repeatedly. | Worst-case complexity is visible. |
| 3 | Evaluate pool limits, material sharing, and spawn density. | Memory retention and physics work increase with the workload. | Retention/creation behavior is visible; exact costs need measurement. |
| Later | Consider asynchronous persistence and a gated transition flow. | Enables responsive presentation and larger saves without blocking the game thread. | A design alternative, not a demonstrated need for this tiny save. |

## 1. Pool searches can become quadratic across a batch

Read [FeatureBallSpawner.cpp](../Source/TestFeatures/Features_NonUE/FeatureBallSpawner.cpp), especially `SpawnMultiple()`, `Spawn()`, and `FindAvailableBall()`.

Every request scans the list from its beginning. Creating an initial batch of N active balls examines about `0 + 1 + ... + (N - 1)` existing entries before deciding to create more: **O(N squared)** search work over that batch. This excludes actor creation itself. Reusing many inactive balls can show a similar pattern as early entries become active again.

At five balls the bookkeeping is small. At large counts, an available-entry stack or queue could make acquiring a reusable ball **O(1) amortized**, while the existing creation-order list still supplies stable names. A release event could return a ball to that list without making the ball know the spawner class.

Tradeoff: an available list adds state to maintain. It must not admit duplicate releases, destroyed entries, or still-active actors. Measure the scan before adding this bookkeeping. Reserving TArray capacity can reduce array reallocations during known bursts, but does not fix repeated scans or actor creation cost.

## 2. Pooling trades creation work for retained memory

The pool has no cap or trimming policy. A temporary peak in demand can leave many inactive world actors for the rest of the session. Weak pool references do not keep actors alive by themselves, but hiding a world actor does not destroy it either. Components and per-ball physical-material subobjects remain allocated.

If actors are destroyed externally, invalid weak entries remain in the creation-order list. They do not keep destroyed actors alive, but they retain list slots and increase future scan work. Stable names currently depend on list length, so simply removing slots would also change naming semantics.

Possible improvements:

- Track active, available, invalid, and total-created counts before choosing a memory policy.
- Separate stable identifiers from array positions if compaction becomes useful.
- Trim unusually large idle pools at a safe transition point, rather than destroying actors on every release.
- Share a physical material if all balls truly use identical settings. Today each ball creates its own material; the mesh asset is already shared.

Prewarming a pool can move creation work away from gameplay bursts, but increases startup work and initial memory. It is not free. The current initial batch is created once, so pooling only pays for reuse when later requests actually occur.

## 3. Physics and bounce meaning deserve attention before micro-optimizations

Read `ConfigureBouncing()`, `Activate()`, and `OnBallHit()` in [FeatureBall.cpp](../Source/TestFeatures/Features/Gameplay/FeatureBall.cpp).

Active physics bodies and dense contact pairs may dominate a large-ball scenario. `AlwaysSpawn` plus independent random positions allows overlapping starts. Those overlaps can cause immediate contact resolution and hit notifications. Collision filtering, a larger spawn volume, or spaced placement could reduce unnecessary contacts if the intended behavior permits it.

The counter measures accepted nonzero-impulse hit callbacks, not a separately detected complete visual rebound. Multiple contact notifications or ball-to-ball impacts can consume the two-hit lifetime quickly. Both balls can legitimately contribute to the shared total for one collision pair under the current definition.

Before optimizing, decide whether the rule means every impact, floor-only rebounds, or distinct separated contacts. A contact-separation rule or minimum impact criterion would change behavior and needs tests; a time cooldown can wrongly discard real rapid impacts.

Earlier offscreen testing reported physics-material access errors during native class-default-object construction. `ConfigureBouncing()` still assigns the material from the constructor. Investigate the exact triggering calls and move engine-dependent runtime application to an appropriate initialized lifecycle point if confirmed. This is a previously observed diagnostic, not a new reproduction or a measured performance bottleneck.

## 4. Events are appropriate here, but their listeners are synchronous

The bounce path is ball -> game-state controller -> shared state -> HUD. Native delegate dispatch and the counter increment are likely modest for this small listener set, but this review does not quantify them. Keeping the event boundary is useful; removing it solely to save a function call would spread dependencies again.

Listeners execute on the publisher's call path. Expensive future listeners will extend the collision handler. The save coordinator also invokes UI listeners synchronously: `OnBeforeIO` is an explicit handshake that lets UI painting and preview waiting complete before disk access starts. Thus the coordinator contains no rendering code, but its elapsed call duration still includes UI work.

If bounce rates become high, coalesce only the **display refreshes** at a chosen cadence while recording every gameplay bounce immediately. That trades visual freshness for less text/layout work without losing the total. It is not needed merely because the design uses delegates.

Subscription lifetimes should be tested: create once, deactivate/reactivate repeatedly, remove/re-add a HUD, and travel between levels. One event should cause exactly one count update and one update per attached HUD. Normal code intends this already; public observation methods could still be called repeatedly by future callers.

## 5. Logging may outweigh small pieces of gameplay bookkeeping

Spawn, activation, movement, bounce, controller handling, state updates, and lifecycle steps produce logs. When these messages are enabled, formatting names, vectors, integers, and writing output has a cost. A thousand events per second is a different workload from five short-lived balls.

An improvement idea is to keep lifecycle summaries at ordinary verbosity and move per-ball detail to a more verbose level or separately configurable category. Summarize aggregate activity during stress tests. Do not remove diagnostic logs before using them to validate behavior, and do not claim disabled logs necessarily evaluate all message expressions: macro and build filtering matter.

Compare the same scene with detailed logging enabled and reduced before spending time on accessor or pointer micro-optimizations.

## 6. Diagnostic history is unbounded and repeatedly reformatted

Read `PublishStatus()` in [FeatureSaveGameSubsystem.cpp](../Source/TestFeatures/Features/GameWorkflow/FeatureSaveGameSubsystem.cpp) and `FormatSaveDiagnostics()` in [FeatureSaveGamePanel.cpp](../Source/TestFeatures/Features/UI/Widgets/FeatureSaveGamePanel.cpp).

Each phase appends an FString to a game-instance-lifetime array. Every refresh joins the entire history and recomputes the desktop path. For H similarly sized entries, one rebuild is **O(H)** text work. Rebuilding after each append can accumulate **O(H squared)** work over a long session. Actual string lengths also matter.

The HUD and modal can each request formatting for a transition. The HUD uses one large wrapped text block inside a scroll box: scrolling does not virtualize the text or bound stored history. The fixed-size modal can clip long diagnostic content; showing the current state first helps but does not make every historical line visible there.

Ideas: keep current state/result separately, retain a bounded recent-history ring, and send the full history to logs when needed. Cache stable slot/path metadata in a diagnostic snapshot. Consider a virtualized list only if histories are genuinely large enough to justify extra UI classes. Prefer one shared UI formatter or diagnostic view model over making the modal depend on the concrete HUD class purely for a static formatting function.

## 7. UI is event-driven, but text changes still cost layout work

`SetTotalBounces()` uses FText formatting on each accepted bounce. Diagnostic changes can replace a longer wrapped string. These actions can invalidate text measurement/layout; event-driven does not mean zero rendering cost.

The current small hierarchy is reasonable for a debug panel. A large canvas or one additional helper function is not automatically a bottleneck. Profile Slate/UMG updates before introducing retainer textures or several invalidation layers; caching and offscreen rendering also have memory and update costs.

The 700 x 290 HUD and 760 x 260 modal should be checked at small resolutions and UI scales. Layout clipping is first a usability issue, not proof of slow rendering. Keep long file paths and diagnostic histories readable while avoiding frequent rebuilds of the widget tree.

## 8. Preview sleeps and render flushes are deliberate stalls

`PresentPhaseForPreview()` calls Slate Tick, renderer Sync, FlushRenderingCommands, and `Sleep(1.0f)`. It runs before I/O and again when the result arrives. Disk timing should be measured **inside storage**, separately from these UI delays. Otherwise a fast save will misleadingly appear to take about two seconds.

For everyday use, make the teaching preview optional while retaining its default when demonstrating task 16. For responsive progress, schedule a UI hold across frames instead of sleeping on the game thread. That requires an explicit loading/saving phase that blocks gameplay and defers normal travel/exit until completion; simply replacing Sleep with a timer would let the current synchronous caller continue too early.

Manual Slate ticks and rendering flushes from initialization or EndPlay should be exercised in PIE, standalone, level travel, and editor stop. Check for reentrancy, pending window destruction, unavailable viewports, and renderer shutdown ordering. These are lifecycle concerns to verify, not asserted crashes in this review. An engine-supported loading-screen flow may be appropriate for a packaged game, but evaluate editor support before replacing a solution intended to work in both modes.

## 9. Save reliability is more urgent than saving a few allocations

The snapshot currently contains one integer. Allocating a USaveGame object once at shutdown is unlikely to be worth complicating the design without measurements. Synchronous startup/shutdown I/O is a simple tradeoff at this payload size; asynchronous APIs are an option for larger payloads, not an automatic improvement.

Current behaviors to review:

- A failed read falls back to zero. A later save can overwrite the same slot with that new total. Decide whether unreadable saves should instead be preserved for recovery.
- A write failure is reported, but normal teardown continues. Decide whether task requirements mean “operation finished” or “successfully persisted”; these are not equivalent.
- EndPlay is already teardown. An asynchronous save cannot safely be started there without retaining the required data and deferring the transition earlier.
- All PIE sessions and levels share one slot and user index. Concurrent instances may overwrite one another. Session-specific development slots or explicit save ownership may be needed.
- The displayed absolute path describes the default desktop backend. It is not a portable guarantee for cloud/platform save systems.
- Abrupt process termination cannot run the normal save path. A checkpoint strategy would improve recovery, at the cost of more writes.

These are policy and lifecycle questions, not reasons to add a large defensive framework to the sandbox. Agree on expected behavior before implementing retries, backups, schema versions, or asynchronous coordination.

## Measurement and validation plan

Keep this as a proposed plan; it was not executed during this review.

1. Validate functionality first: one bounce -> one total increment; two accepted hits -> inactive ball; reuse -> reset local count; HUD remove/re-add -> no duplicate callbacks; load -> correct initial HUD; save failure -> agreed exit behavior.
2. Test the default count, then controlled larger counts such as 50 and 500 if the machine can support them. These are test points, not promised supported capacities. Keep resolution, spawn volume, and logging settings recorded.
3. Measure initial creation separately from reusing an already-filled pool. Capture search probes, active/idle counts, pool high-water mark, actor allocations, and retained memory.
4. Compare sparse and deliberately dense spawn volumes to identify collision pressure rather than attributing every slowdown to the pool.
5. Trace gameplay CPU, physics, Slate work, and allocations. Separate first-load shader/asset work from steady-state operation and editor work from standalone runs.
6. Measure real load/save calls separately from the one-second preview holds and render synchronization. Exercise missing/unreadable saves and failed writes in an isolated test slot, protecting real progress.
7. Change one thing at a time; record timings and behavior before/after. Use average and tail frame times, not FPS alone.

### Unreal documentation for deeper investigation

- [Unreal Insights](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine): capture timelines and inspect where time is spent.
- [UMG optimization guidelines](https://dev.epicgames.com/documentation/en-us/unreal-engine/optimization-guidelines-for-umg-in-unreal-engine): event-driven updates, layout invalidation, and widget costs.
- [Saving and loading](https://dev.epicgames.com/documentation/en-us/unreal-engine/saving-and-loading-your-game-in-unreal-engine): SaveGame snapshots and synchronous/asynchronous persistence options.

## What not to optimize yet

Do not replace the event architecture with direct gameplay-to-UI calls, introduce Mass solely for five actors, build a generic pool framework before its contract is clear, or replace readable value types with pointers to avoid tiny copies. Class/file separation changes organization, not inherently frame time. Preserve the teaching value and fix measured costs or verified lifecycle problems first.
