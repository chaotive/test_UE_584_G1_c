#if WITH_DEV_AUTOMATION_TESTS

#include "FeatureGameState.h"
#include "FeatureBall.h"
#include "Features/GameWorkflow/FeatureGameplaySession.h"
#include "Features/GameWorkflow/FeatureSaveGame.h"
#include "Features/UI/Views/FeatureMenuViews.h"
#include "Features_NonUE/FeatureBallSpawner.h"
#include "Features_GameLogic/FeatureLevelRules.h"
#include "Engine/World.h"
#include "Engine/HitResult.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "TimerManager.h"

// Initializes a fresh state fixture without involving persistence or a game instance.
// State is borrowed and Level is copied; the state copies the helper's local progress values.
static void InitializeFreshSession(AFeatureGameState& State, int32 Level)
{
	FFeatureGameProgress Progress;
	Progress.CurrentLevel = Level;
	State.InitializeSession(Progress);
}

// Checks the actions and Escape behavior exposed by each menu view.
// The automation framework owns the test instance; no pointer or reference ownership is transferred.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeatureMenuSmokeTest, "Feature.Smoke.MenuViews",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFeatureMenuSmokeTest::RunTest(const FString& Parameters)
{
	const FFeatureMenuView Initial = FFeatureMenuViews::ForFlow(EFeatureGameFlow::StartupMenu);
	const FFeatureMenuView Pause = FFeatureMenuViews::ForFlow(EFeatureGameFlow::PauseMenu);
	const FFeatureMenuView Victory = FFeatureMenuViews::ForFlow(EFeatureGameFlow::Victory);
	const FFeatureMenuView Defeat = FFeatureMenuViews::ForFlow(EFeatureGameFlow::Lose);

	if (!TestEqual(TEXT("Initial action count"), Initial.Items.Num(), 4)
		|| !TestEqual(TEXT("Pause action count"), Pause.Items.Num(), 3)
		|| !TestEqual(TEXT("Victory action count"), Victory.Items.Num(), 2)
		|| !TestEqual(TEXT("Defeat action count"), Defeat.Items.Num(), 2))
	{
		return false;
	}

	TestTrue(TEXT("Initial menu offers New Game first"), Initial.Items[0].Action == EFeatureMenuAction::NewGame);
	TestTrue(TEXT("Initial menu offers Continue"), Initial.Items[1].Action == EFeatureMenuAction::Continue);
	TestTrue(TEXT("Initial menu offers Delete Save before Exit"), Initial.Items[2].Action == EFeatureMenuAction::DeleteSave
		&& Initial.Items[3].Action == EFeatureMenuAction::Exit);
	TestTrue(TEXT("Escape exits from the initial menu"), Initial.bExitOnEscape && !Initial.bPauseMenu);
	TestTrue(TEXT("Pause menu starts with Resume"), Pause.Items[0].Action == EFeatureMenuAction::Resume && Pause.bPauseMenu);
	TestTrue(TEXT("Victory starts the saved next level"), Victory.Items[0].Action == EFeatureMenuAction::Retry);
	TestTrue(TEXT("Defeat offers Retry and Exit"), Defeat.Items[0].Action == EFeatureMenuAction::Retry
		&& Defeat.Items[1].Action == EFeatureMenuAction::Exit);
	return true;
}

// Checks fresh session counters, outcome recording, and level advancement.
// The automation framework owns the test instance; its temporary world owns the game-state actors.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeatureSessionSmokeTest, "Feature.Smoke.SessionState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFeatureSessionSmokeTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AFeatureGameState* State = World->SpawnActor<AFeatureGameState>();
	TestFalse(TEXT("Newly created state has no initialized session"), State->IsSessionInitialized());
	InitializeFreshSession(*State, 1);
	TestTrue(TEXT("Supplying progress initializes the session"), State->IsSessionInitialized());
	State->RecordBounce();
	State->RecordPlayerBallCollision();

	TestEqual(TEXT("Fresh session starts at level one"), State->GetCurrentLevel(), 1);
	TestEqual(TEXT("Bounce updates aggregate"), State->GetTotalBounces(), 1);
	TestEqual(TEXT("Bounce updates active session"), State->GetGameplaySessions().Last().TotalBounces, 1);
	TestEqual(TEXT("Player contact updates active session"), State->GetGameplaySessions().Last().PlayerBallCollisions, 1);

	State->RecordSessionResult(true);
	State->RecordSessionResult(true);
	State->RecordBounce();
	TestEqual(TEXT("Victory advances the level once"), State->GetCurrentLevel(), 2);
	TestEqual(TEXT("Ended sessions stop counting bounces"), State->GetTotalBounces(), 1);
	TestTrue(TEXT("Victory is recorded"), State->GetGameplaySessions().Last().Result == EFeatureGameplaySessionResult::Victory);

	AFeatureGameState* DefeatState = World->SpawnActor<AFeatureGameState>();
	InitializeFreshSession(*DefeatState, 3);
	DefeatState->RecordSessionResult(false);
	TestEqual(TEXT("Defeat retains a later level"), DefeatState->GetCurrentLevel(), 3);
	TestTrue(TEXT("Defeat is recorded"), DefeatState->GetGameplaySessions().Last().Result == EFeatureGameplaySessionResult::Defeat);

	FFeatureGameProgress RestoredProgress;
	RestoredProgress.CurrentLevel = 4;
	RestoredProgress.TotalBounces = 21;
	RestoredProgress.Sessions.AddDefaulted_GetRef().Result = EFeatureGameplaySessionResult::Victory;
	State->InitializeSession(RestoredProgress);
	TestEqual(TEXT("Initialization updates the borrowed snapshot for save-after-load"), RestoredProgress.Sessions.Num(), 2);
	TestEqual(TEXT("State retains the initialized session history"), State->GetProgress().Sessions.Num(), 2);
	TestEqual(TEXT("Restored aggregate remains authoritative"), State->GetTotalBounces(), 21);
	World->DestroyWorld(false);
	return true;
}

// Checks script-backed outcomes, precedence, and thresholds across progression levels.
// The automation framework owns the test; its temporary world owns the game-state actor and subsystem.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeatureLossLimitSmokeTest, "Feature.Smoke.SessionLossLimit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFeatureLossLimitSmokeTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AFeatureGameState* State = World->SpawnActor<AFeatureGameState>();
	UFeatureGameplaySession* Session = World->GetSubsystem<UFeatureGameplaySession>();
	Session->GameState = State;
	Session->PlayerContactsToWin = 10;
	Session->ContactsIncreasePerLevel = 1;
	Session->SessionBouncesToLose = 100;
	Session->BouncesDecreasePerLevel = 1;
	Session->MinimumSessionBouncesToLose = 50;
	UFeatureLevelRules* Rules = World->GetSubsystem<UFeatureLevelRules>();
	TestTrue(TEXT("Below both limits the session remains in progress"),
		Rules->SessionOutcome(9, 99, 10, 100) == EFeatureGameplaySessionResult::InProgress);
	TestTrue(TEXT("Reaching the contact limit chooses victory"),
		Rules->SessionOutcome(10, 99, 10, 100) == EFeatureGameplaySessionResult::Victory);
	TestTrue(TEXT("Reaching the bounce limit chooses defeat"),
		Rules->SessionOutcome(9, 100, 10, 100) == EFeatureGameplaySessionResult::Defeat);
	TestTrue(TEXT("Victory takes precedence when both limits are reached"),
		Rules->SessionOutcome(10, 100, 10, 100) == EFeatureGameplaySessionResult::Victory);
	TestEqual(TEXT("An unfinished result retains the current level"),
		Rules->NextLevel(9, EFeatureGameplaySessionResult::InProgress), 9);

	InitializeFreshSession(*State, 1);
	TestEqual(TEXT("The script returns the configured level-one win target"), Session->GetPlayerContactsToWin(), 10);
	TestEqual(TEXT("Level one uses the new 100-bounce base target"), Session->GetSessionBouncesToLose(), 100);
	InitializeFreshSession(*State, 2);
	TestEqual(TEXT("The script increases the win target at level two"), Session->GetPlayerContactsToWin(), 11);
	TestEqual(TEXT("The target decreases by one at level two"), Session->GetSessionBouncesToLose(), 99);
	InitializeFreshSession(*State, 52);
	TestEqual(TEXT("The target stops at the configured 50-bounce minimum"), Session->GetSessionBouncesToLose(), 50);

	World->DestroyWorld(false);
	return true;
}

// Checks that player contact retires the ball and routes it through the replacement pool.
// The automation framework owns the test; the temporary world owns the pawn, controller, and pooled balls.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeaturePlayerBallReplacementSmokeTest, "Feature.Smoke.PlayerBallReplacement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFeaturePlayerBallReplacementSmokeTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	UFeatureGameplaySession* Session = World->GetSubsystem<UFeatureGameplaySession>();
	Session->SessionState = UFeatureGameplaySession::ESessionState::Playing;
	Session->BallSpawnBounds = FBox(FVector(-100.0f), FVector(100.0f));
	Session->BallSpawner.Prewarm(*World, 1);
	AFeatureBall* Ball = Session->BallSpawner.Spawn(*World, FVector(0.0f, 0.0f, 500.0f));
	Session->ObserveBall(*Ball);
	APawn* PlayerPawn = World->SpawnActor<APawn>();
	APlayerController* PlayerController = World->SpawnActor<APlayerController>();
	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	PlayerController->Possess(PlayerPawn);
	TestTrue(TEXT("The possessed pawn has a non-bot player state"), PlayerPawn->IsPlayerControlled());
	bool bPlayerContactAnnounced = false;
	Session->OnPlayerBallContact.AddLambda([&bPlayerContactAnnounced, PlayerPawn](APawn& ContactedPawn)
	{
		bPlayerContactAnnounced = &ContactedPawn == PlayerPawn;
	});

	Ball->OnBallHit(nullptr, PlayerPawn, nullptr, FVector::ZeroVector, FHitResult());
	TestTrue(TEXT("Player contact is published"), bPlayerContactAnnounced);
	TestFalse(TEXT("The contacted ball is retired"), Ball->IsActive());
	TestEqual(TEXT("Retirement schedules one replacement"), Session->PendingBallReplacements, 1);
	TestEqual(TEXT("A swept player contact does not count as a non-player bounce"), Ball->BounceCount, 0);
	World->GetTimerManager().Tick(0.01f);
	TestTrue(TEXT("The replacement reuses the retired ball"), Ball->IsActive());
	TestEqual(TEXT("Replacement does not grow the prewarmed pool"), Session->BallSpawner.GetSpawnedBalls().Num(), 1);
	TestEqual(TEXT("The scheduled replacement is consumed"), Session->PendingBallReplacements, 0);

	Ball->Configure(1.0f, 1.0f, 2);
	Ball->OnBallHit(nullptr, nullptr, nullptr, FVector::ZeroVector, FHitResult());
	TestEqual(TEXT("A non-player contact without solver impulse is ignored"), Ball->BounceCount, 0);
	TestTrue(TEXT("An ignored contact leaves the ball active"), Ball->IsActive());
	Ball->OnBallHit(nullptr, nullptr, nullptr, FVector::UpVector, FHitResult());
	TestEqual(TEXT("A solver impact counts one bounce"), Ball->BounceCount, 1);
	TestTrue(TEXT("The ball remains active below its rebound limit"), Ball->IsActive());
	Ball->OnBallHit(nullptr, nullptr, nullptr, FVector::UpVector, FHitResult());
	TestEqual(TEXT("The rebound limit counts the last bounce"), Ball->BounceCount, 2);
	TestFalse(TEXT("The scripted rebound limit retires the ball"), Ball->IsActive());
	TestEqual(TEXT("Non-player retirement also schedules replacement"), Session->PendingBallReplacements, 1);

	Session->SessionState = UFeatureGameplaySession::ESessionState::Paused;
	Session->SpawnReplacementBall();
	TestFalse(TEXT("The script defers replacement while paused"), Ball->IsActive());
	TestEqual(TEXT("Deferred replacement remains queued"), Session->PendingBallReplacements, 1);
	Session->SessionState = UFeatureGameplaySession::ESessionState::Playing;
	Session->SpawnReplacementBall();
	TestTrue(TEXT("Active gameplay permits replacement again"), Ball->IsActive());
	Session->SessionState = UFeatureGameplaySession::ESessionState::Ended;
	Ball->Deactivate();
	TestEqual(TEXT("Retirement after the session ends schedules no replacement"), Session->PendingBallReplacements, 0);

	World->DestroyWorld(false);
	return true;
}

// Checks that prewarmed balls are activated and reused rather than duplicated.
// The test owns the spawner by value; the temporary world owns its ball actors.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeatureBallPoolSmokeTest, "Feature.Smoke.BallPool",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFeatureBallPoolSmokeTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FFeatureBallSpawner Spawner;
	Spawner.Prewarm(*World, 3);
	TestEqual(TEXT("Prewarm creates the requested pool size"), Spawner.GetSpawnedBalls().Num(), 3);

	AFeatureBall* First = Spawner.Spawn(*World, FVector(0.0f, 0.0f, 500.0f));
	TestTrue(TEXT("Spawn activates a prewarmed ball"), First->IsActive());
	TestEqual(TEXT("Spawn reuses the prewarmed pool"), Spawner.GetSpawnedBalls().Num(), 3);
	First->Deactivate();
	AFeatureBall* Reused = Spawner.Spawn(*World, FVector(0.0f, 0.0f, 600.0f));
	TestTrue(TEXT("Retired ball is reused"), Reused == First && Reused->IsActive());
	World->DestroyWorld(false);
	return true;
}

// Checks that saved level and session statistics survive Unreal serialization.
// The test owns its byte array by value; Unreal garbage collection owns both save objects.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeatureSaveSmokeTest, "Feature.Smoke.SaveRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFeatureSaveSmokeTest::RunTest(const FString& Parameters)
{
	UFeatureSaveGame* Original = NewObject<UFeatureSaveGame>();
	Original->Progress.CurrentLevel = 3;
	Original->Progress.TotalBounces = 14;
	FFeatureGameplaySessionRecord& Session = Original->Progress.Sessions.AddDefaulted_GetRef();
	Session.TotalBounces = 4;
	Session.PlayerBallCollisions = 2;
	Session.Result = EFeatureGameplaySessionResult::Defeat;

	TArray<uint8> Bytes;
	if (!TestTrue(TEXT("Save serializes to memory"), UGameplayStatics::SaveGameToMemory(Original, Bytes)))
	{
		return false;
	}
	UFeatureSaveGame* Restored = Cast<UFeatureSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
	if (!TestNotNull(TEXT("Save deserializes from memory"), Restored))
	{
		return false;
	}
	TestEqual(TEXT("Saved level survives"), Restored->Progress.CurrentLevel, 3);
	TestEqual(TEXT("Aggregate bounces survive"), Restored->Progress.TotalBounces, 14);
	if (TestEqual(TEXT("Session history survives"), Restored->Progress.Sessions.Num(), 1))
	{
		const FFeatureGameplaySessionRecord& LoadedSession = Restored->Progress.Sessions[0];
		TestEqual(TEXT("Session bounces survive"), LoadedSession.TotalBounces, 4);
		TestEqual(TEXT("Player contacts survive"), LoadedSession.PlayerBallCollisions, 2);
		TestTrue(TEXT("Session result survives"), LoadedSession.Result == EFeatureGameplaySessionResult::Defeat);
	}
	return true;
}

#endif
