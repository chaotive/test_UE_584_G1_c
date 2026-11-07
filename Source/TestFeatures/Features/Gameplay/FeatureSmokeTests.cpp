#if WITH_DEV_AUTOMATION_TESTS

#include "FeatureGameState.h"
#include "FeatureBall.h"
#include "Features/GameWorkflow/FeatureSaveGame.h"
#include "Features/UI/Views/FeatureMenuViews.h"
#include "Features_NonUE/FeatureBallSpawner.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"

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
	State->StartSession(false, 1);
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
	DefeatState->StartSession(false, 1);
	DefeatState->RecordSessionResult(false);
	TestEqual(TEXT("Defeat retains the level"), DefeatState->GetCurrentLevel(), 1);
	TestTrue(TEXT("Defeat is recorded"), DefeatState->GetGameplaySessions().Last().Result == EFeatureGameplaySessionResult::Defeat);
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
