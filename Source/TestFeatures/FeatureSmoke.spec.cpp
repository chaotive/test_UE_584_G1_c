#if WITH_DEV_AUTOMATION_TESTS

#include "Features/Gameplay/Actors/FeatureBall.h"
#include "Features/Gameplay/FeatureBallManager.h"
#include "Features/Gameplay/FeatureGameState.h"
#include "Features/Gameplay/FeatureGameStateController.h"
#include "Features/GameWorkflow/FeatureGameFlow.h"
#include "Features/GameWorkflow/FeatureGameplaySession.h"
#include "Features/GameWorkflow/FeatureSaveGame.h"
#include "Features/UI/Controllers/FeatureMenuActions.h"
#include "Features/UI/Views/FeatureMenuViews.h"
#include "Features_GameLogic/FeatureLevelRules.h"
#include "Containers/Array.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Misc/DateTime.h"
#include "TimerManager.h"

// Starts a session with empty progress at the level selected by the scenario.
// Borrows the state actor, which copies the local snapshot.
static void StartFreshSession(AFeatureGameState& State, int32 Level)
{
	FFeatureGameProgress Progress;
	Progress.CurrentLevel = Level;
	State.InitializeSession(Progress);
}

// Defines smoke scenarios shared by the feature module.
// Unreal registers this spec under the Feature.Smoke filter.
BEGIN_DEFINE_SPEC(FFeatureSmokeSpec, "Feature.Smoke.Features",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
	// Defines the menu action and Escape scenarios.
	void DefineMenus();

	// Defines the scoring, progression, and outcome scenarios.
	void DefineSessions();

	// Defines the contact, retirement, and pool reuse scenarios.
	void DefineBalls();

	// Defines the in-memory save history scenario.
	void DefineSaveHistory();

	// Checks the complete menu action order.
	// Borrows the view and the expected list for this assertion.
	void ExpectMenuActions(const FFeatureMenuView& View, const TArray<EFeatureMenuAction>& Expected);

	// Sends a contact through the same handler used by the ball's hit event.
	// Borrows the ball and impulse. OtherActor is null for a contact without a fixture actor.
	void HitBall(AFeatureBall& Ball, AActor* OtherActor, const FVector& Impulse);

	// Owns the temporary world used by the current gameplay scenario.
	// The world is rooted by Unreal and destroyed in AfterEach.
	UWorld* World = nullptr;
END_DEFINE_SPEC(FFeatureSmokeSpec)

void FFeatureSmokeSpec::Define()
{
	DefineMenus();
	DefineSaveHistory();

	Describe("Gameplay", [this]()
	{
		BeforeEach([this]()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
		});

		DefineSessions();
		DefineBalls();

		AfterEach([this]()
		{
			World->DestroyWorld(false);
			World = nullptr;
		});
	});
}

void FFeatureSmokeSpec::DefineMenus()
{
	Describe("Menu views", [this]()
	{
		It("offers start and save actions on the startup menu", [this]()
		{
			const FFeatureMenuView Menu = FFeatureMenuViews::ForFlow(EFeatureGameFlow::StartupMenu);
			ExpectMenuActions(Menu, {EFeatureMenuAction::NewGame, EFeatureMenuAction::Continue,
				EFeatureMenuAction::DeleteSave, EFeatureMenuAction::Exit});
			TestTrue("Escape exits the game", Menu.bExitOnEscape);
			TestFalse("Escape does not resume from startup", Menu.bPauseMenu);
		});

		It("offers resume, main menu, and exit when paused", [this]()
		{
			const FFeatureMenuView Menu = FFeatureMenuViews::ForFlow(EFeatureGameFlow::PauseMenu);
			ExpectMenuActions(Menu, {EFeatureMenuAction::Resume, EFeatureMenuAction::ReturnToMenu, EFeatureMenuAction::Exit});
			TestTrue("Escape resumes play", Menu.bPauseMenu);
		});

		It("offers the saved next level and exit after victory", [this]()
		{
			const FFeatureMenuView Menu = FFeatureMenuViews::ForFlow(EFeatureGameFlow::Victory);
			ExpectMenuActions(Menu, {EFeatureMenuAction::Retry, EFeatureMenuAction::Exit});
			TestFalse("Escape does not resume or exit from victory", Menu.bPauseMenu || Menu.bExitOnEscape);
		});

		It("offers a retry and exit after defeat", [this]()
		{
			const FFeatureMenuView Menu = FFeatureMenuViews::ForFlow(EFeatureGameFlow::Lose);
			ExpectMenuActions(Menu, {EFeatureMenuAction::Retry, EFeatureMenuAction::Exit});
			TestFalse("Escape does not resume or exit from defeat", Menu.bPauseMenu || Menu.bExitOnEscape);
		});
	});
}

void FFeatureSmokeSpec::ExpectMenuActions(const FFeatureMenuView& View, const TArray<EFeatureMenuAction>& Expected)
{
	TArray<EFeatureMenuAction> Actions;
	for (const FFeatureMenuActionDefinition& Item : View.Items)
	{
		Actions.Add(Item.Action);
	}
	TestEqual("Actions in display order", Actions, Expected);
}

void FFeatureSmokeSpec::DefineSessions()
{
	Describe("Session state", [this]()
	{
		It("starts at level one and tracks bounces and player contacts", [this]()
		{
			AFeatureGameState& State = *World->SpawnActor<AFeatureGameState>();
			TestFalse("A new state has no active session", State.IsSessionInitialized());

			StartFreshSession(State, 1);
			State.RecordBounce();
			State.RecordPlayerBallCollision();

			TestTrue("Progress initializes the session", State.IsSessionInitialized());
			TestEqual("Current level", State.GetCurrentLevel(), 1);
			TestEqual("Aggregate bounces", State.GetTotalBounces(), 1);
			const FFeatureGameplaySessionRecord& Session = State.GetGameplaySessions().Last();
			TestEqual("Session bounces", Session.TotalBounces, 1);
			TestEqual("Session player contacts", Session.PlayerBallCollisions, 1);
		});

		It("advances once on victory and stops counting bounces", [this]()
		{
			AFeatureGameState& State = *World->SpawnActor<AFeatureGameState>();
			StartFreshSession(State, 1);
			State.RecordBounce();

			State.RecordSessionResult(true);
			State.RecordSessionResult(true);
			State.RecordBounce();

			TestEqual("Current level", State.GetCurrentLevel(), 2);
			TestEqual("Aggregate bounces", State.GetTotalBounces(), 1);
			TestEqual("Session result", State.GetGameplaySessions().Last().Result, EFeatureGameplaySessionResult::Victory);
		});

		It("keeps the current level on defeat", [this]()
		{
			AFeatureGameState& State = *World->SpawnActor<AFeatureGameState>();
			StartFreshSession(State, 3);
			State.RecordSessionResult(false);

			TestEqual("Current level", State.GetCurrentLevel(), 3);
			TestEqual("Session result", State.GetGameplaySessions().Last().Result, EFeatureGameplaySessionResult::Defeat);
		});

		It("adds an active session to the restored history", [this]()
		{
			AFeatureGameState& State = *World->SpawnActor<AFeatureGameState>();
			FFeatureGameProgress Progress;
			Progress.CurrentLevel = 4;
			Progress.TotalBounces = 21;
			Progress.Sessions.AddDefaulted_GetRef().Result = EFeatureGameplaySessionResult::Victory;

			State.InitializeSession(Progress);

			TestEqual("History available for a save after load", Progress.Sessions.Num(), 2);
			TestEqual("History retained by game state", State.GetGameplaySessions().Num(), 2);
			TestEqual("Restored aggregate bounces", State.GetTotalBounces(), 21);
		});
	});

	Describe("Outcome rules", [this]()
	{
		It("checks both limits and gives victory precedence", [this]()
		{
			const UFeatureLevelRules& Rules = *World->GetSubsystem<UFeatureLevelRules>();
			TestEqual("Below both limits", Rules.SessionOutcome(9, 99, 10, 100), EFeatureGameplaySessionResult::InProgress);
			TestEqual("Contact limit reached", Rules.SessionOutcome(10, 99, 10, 100), EFeatureGameplaySessionResult::Victory);
			TestEqual("Bounce limit reached", Rules.SessionOutcome(9, 100, 10, 100), EFeatureGameplaySessionResult::Defeat);
			TestEqual("Both limits reached", Rules.SessionOutcome(10, 100, 10, 100), EFeatureGameplaySessionResult::Victory);
			TestEqual("An unfinished session keeps its level", Rules.NextLevel(9, EFeatureGameplaySessionResult::InProgress), 9);
		});

		It("raises the win target and lowers the loss target to its floor", [this]()
		{
			AFeatureGameState& State = *World->SpawnActor<AFeatureGameState>();
			UFeatureGameplaySession& Session = *World->GetSubsystem<UFeatureGameplaySession>();
			Session.GameState = &State;
			Session.PlayerContactsToWin = 10;
			Session.ContactsIncreasePerLevel = 1;
			Session.SessionBouncesToLose = 100;
			Session.BouncesDecreasePerLevel = 1;
			Session.MinimumSessionBouncesToLose = 50;

			StartFreshSession(State, 1);
			TestEqual("Level one win target", Session.GetPlayerContactsToWin(), 10);
			TestEqual("Level one loss target", Session.GetSessionBouncesToLose(), 100);

			StartFreshSession(State, 2);
			TestEqual("Level two win target", Session.GetPlayerContactsToWin(), 11);
			TestEqual("Level two loss target", Session.GetSessionBouncesToLose(), 99);

			StartFreshSession(State, 52);
			TestEqual("Minimum loss target", Session.GetSessionBouncesToLose(), 50);
		});
	});
}

void FFeatureSmokeSpec::DefineBalls()
{
	Describe("Ball lifecycle", [this]()
	{
		It("retires on player contact and keeps one scoring subscription after reuse", [this]()
		{
			UFeatureBallManager& Manager = *World->GetSubsystem<UFeatureBallManager>();
			AFeatureGameState& State = *World->SpawnActor<AFeatureGameState>();
			StartFreshSession(State, 1);
			UFeatureGameStateController* Scoring = NewObject<UFeatureGameStateController>(&State);
			Scoring->Initialize(State);
			Manager.OnBallCreated.AddUObject(Scoring, &UFeatureGameStateController::ObserveBall);
			Manager.Start(1, 1.0f, 1.0f, 2, FBox(FVector(-100.0f), FVector(100.0f)));
			AFeatureBall& Ball = *Manager.GetSpawnedBalls()[0].Get();

			APawn* Player = World->SpawnActor<APawn>();
			APlayerController* Controller = World->SpawnActor<APlayerController>();
			Controller->PlayerState = World->SpawnActor<APlayerState>();
			Controller->Possess(Player);
			TestTrue("The fixture pawn is player-controlled", Player->IsPlayerControlled());

			bool bContactPublished = false;
			const FDelegateHandle ContactBinding = Manager.OnPlayerBallContact.AddLambda([&bContactPublished, Player](APawn& Pawn)
			{
				bContactPublished = &Pawn == Player;
			});
			HitBall(Ball, Player, FVector::ZeroVector);
			Manager.OnPlayerBallContact.Remove(ContactBinding);

			TestTrue("The manager publishes the contacted pawn", bContactPublished);
			TestEqual("Player contacts", State.GetGameplaySessions().Last().PlayerBallCollisions, 1);
			TestEqual("Aggregate non-player bounces", State.GetTotalBounces(), 0);
			TestEqual("Ball bounces after player contact", Ball.BounceCount, 0);
			TestFalse("The ball retires", Ball.IsActive());
			TestEqual("Queued replacements", Manager.PendingBallReplacements, 1);

			World->GetTimerManager().Tick(0.01f);
			TestTrue("The queued replacement reactivates the same ball", Ball.IsActive());
			TestEqual("Pool size after replacement", Manager.GetSpawnedBalls().Num(), 1);
			TestEqual("Remaining replacements", Manager.PendingBallReplacements, 0);

			HitBall(Ball, nullptr, FVector::UpVector);
			HitBall(Ball, nullptr, FVector::UpVector);
			TestEqual("Two impacts after reuse add two bounces", State.GetTotalBounces(), 2);
		});

		It("counts solver impacts and retires at the rebound limit", [this]()
		{
			UFeatureBallManager& Manager = *World->GetSubsystem<UFeatureBallManager>();
			Manager.Start(1, 1.0f, 1.0f, 2, FBox(FVector(-100.0f), FVector(100.0f)));
			AFeatureBall& Ball = *Manager.GetSpawnedBalls()[0].Get();

			HitBall(Ball, nullptr, FVector::ZeroVector);
			TestEqual("A contact without solver impulse adds no bounce", Ball.BounceCount, 0);
			TestTrue("The ignored contact leaves the ball active", Ball.IsActive());

			HitBall(Ball, nullptr, FVector::UpVector);
			TestEqual("The first impact counts", Ball.BounceCount, 1);
			TestTrue("The ball stays active below its limit", Ball.IsActive());

			HitBall(Ball, nullptr, FVector::UpVector);
			TestEqual("The retiring impact also counts", Ball.BounceCount, 2);
			TestFalse("The ball retires at its limit", Ball.IsActive());
			TestEqual("Queued replacements", Manager.PendingBallReplacements, 1);
		});

		It("defers replacements while paused and clears the queue on stop", [this]()
		{
			UFeatureBallManager& Manager = *World->GetSubsystem<UFeatureBallManager>();
			Manager.Start(1, 1.0f, 1.0f, 2, FBox(FVector(-100.0f), FVector(100.0f)));
			AFeatureBall& Ball = *Manager.GetSpawnedBalls()[0].Get();
			Ball.Deactivate();

			Manager.Pause();
			Manager.SpawnReplacementBall();
			TestFalse("The ball stays inactive while paused", Ball.IsActive());
			TestEqual("The replacement stays queued", Manager.PendingBallReplacements, 1);

			Manager.Resume();
			TestTrue("Resuming activates the replacement", Ball.IsActive());

			Ball.Deactivate();
			TestEqual("Retiring before stop queues a replacement", Manager.PendingBallReplacements, 1);
			Manager.Stop();
			TestEqual("Stopping clears the queue", Manager.PendingBallReplacements, 0);
			Manager.SpawnReplacementBall();
			TestFalse("A stopped manager leaves the ball inactive", Ball.IsActive());
		});

		It("reuses inactive pool entries for explicit spawns", [this]()
		{
			UFeatureBallManager& Manager = *World->GetSubsystem<UFeatureBallManager>();
			Manager.Prewarm(3);
			TestEqual("Prewarmed pool size", Manager.GetSpawnedBalls().Num(), 3);

			AFeatureBall* First = Manager.Spawn(FVector(0.0f, 0.0f, 500.0f));
			TestTrue("Spawning activates a pooled ball", First->IsActive());
			TestEqual("Pool size after spawn", Manager.GetSpawnedBalls().Num(), 3);

			First->Deactivate();
			AFeatureBall* Reused = Manager.Spawn(FVector(0.0f, 0.0f, 600.0f));
			TestTrue("Spawning reactivates the retired actor", Reused == First && Reused->IsActive());
		});
	});
}

void FFeatureSmokeSpec::HitBall(AFeatureBall& Ball, AActor* OtherActor, const FVector& Impulse)
{
	Ball.OnBallHit(nullptr, OtherActor, nullptr, Impulse, FHitResult());
}

void FFeatureSmokeSpec::DefineSaveHistory()
{
	Describe("Save history", [this]()
	{
		It("preserves level, counts, and ordered session results in a round trip", [this]()
		{
			const FDateTime EarlierStart(2026, 11, 8, 12);
			const FDateTime LaterStart(2026, 11, 8, 13);
			UFeatureSaveGame* Save = NewObject<UFeatureSaveGame>();
			Save->Progress.CurrentLevel = 3;
			Save->Progress.TotalBounces = 14;

			FFeatureGameplaySessionRecord& Earlier = Save->Progress.Sessions.AddDefaulted_GetRef();
			Earlier.StartedAtUtc = EarlierStart;
			Earlier.TotalBounces = 10;
			Earlier.Result = EFeatureGameplaySessionResult::Victory;

			FFeatureGameplaySessionRecord& Later = Save->Progress.Sessions.AddDefaulted_GetRef();
			Later.StartedAtUtc = LaterStart;
			Later.TotalBounces = 4;
			Later.PlayerBallCollisions = 2;
			Later.Result = EFeatureGameplaySessionResult::Defeat;

			TArray<uint8> Bytes;
			if (!TestTrue("Serializes to memory", UGameplayStatics::SaveGameToMemory(Save, Bytes)))
			{
				return;
			}
			const UFeatureSaveGame* Loaded = Cast<UFeatureSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
			if (!TestNotNull("Deserializes from memory", Loaded))
			{
				return;
			}

			TestEqual("Saved level", Loaded->Progress.CurrentLevel, 3);
			TestEqual("Aggregate bounces", Loaded->Progress.TotalBounces, 14);
			const TArray<FFeatureGameplaySessionRecord>& Sessions = Loaded->Progress.Sessions;
			if (!TestEqual("Number of saved sessions", Sessions.Num(), 2))
			{
				return;
			}
			TestEqual("First session start", Sessions[0].StartedAtUtc, EarlierStart);
			TestEqual("First session result", Sessions[0].Result, EFeatureGameplaySessionResult::Victory);
			TestEqual("Latest session start", Sessions[1].StartedAtUtc, LaterStart);
			TestEqual("Latest session result", Sessions[1].Result, EFeatureGameplaySessionResult::Defeat);
			TestEqual("Latest session bounces", Sessions[1].TotalBounces, 4);
			TestEqual("Latest session contacts", Sessions[1].PlayerBallCollisions, 2);
		});
	});
}

#endif
