#if WITH_DEV_AUTOMATION_TESTS

#include "test_UE_584_G1_cCharacter.h"
#include "Engine/World.h"
#include "Features_GameLogic/FeaturePlayerRules.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "TimerManager.h"

// Checks base speed, boost refresh, stacking, and expiry across timer-manager frames.
// The automation framework owns the test; the temporary world owns its spawned character.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeaturePlayerSpeedBoostSmokeTest, "Feature.Smoke.PlayerSpeedBoost",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFeaturePlayerSpeedBoostSmokeTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	UClass* CharacterClass = LoadClass<Atest_UE_584_G1_cCharacter>(nullptr,
		TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C"));
	if (!TestNotNull(TEXT("Third-person character Blueprint class loads"), CharacterClass))
	{
		World->DestroyWorld(false);
		return false;
	}

	Atest_UE_584_G1_cCharacter* Character = World->SpawnActor<Atest_UE_584_G1_cCharacter>(CharacterClass);
	UFeaturePlayerRules* Logic = Character->FindComponentByClass<UFeaturePlayerRules>();
	Logic->ConfigureSpeed(0.5f, 2.0f, 5.0f, false);
	TestEqual(TEXT("Base multiplier configures the baseline walk speed"), Character->GetCharacterMovement()->MaxWalkSpeed, 250.0f);
	Logic->HandleBallContact(*Character);
	TestEqual(TEXT("Ball contact temporarily doubles the base speed"), Character->GetCharacterMovement()->MaxWalkSpeed, 500.0f);

	Atest_UE_584_G1_cCharacter* StackingCharacter = World->SpawnActor<Atest_UE_584_G1_cCharacter>(CharacterClass);
	UFeaturePlayerRules* StackingLogic = StackingCharacter->FindComponentByClass<UFeaturePlayerRules>();
	StackingLogic->ConfigureSpeed(0.5f, 2.0f, 5.0f, true);
	StackingLogic->HandleBallContact(*StackingCharacter);
	StackingLogic->HandleBallContact(*StackingCharacter);
	TestEqual(TEXT("Stacking adds one contact boost per contact"), StackingCharacter->GetCharacterMovement()->MaxWalkSpeed, 750.0f);

	// Each incomplete latent update yields an engine frame; timers activate before their elapsed-time checks.
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, World, Character, StackingCharacter, Logic, Step = 0]() mutable
	{
		switch (Step++)
		{
		case 0:
			World->GetTimerManager().Tick(0.0f);
			return false;
		case 1:
			World->GetTimerManager().Tick(4.0f);
			TestEqual(TEXT("The boost remains before expiry"), Character->GetCharacterMovement()->MaxWalkSpeed, 500.0f);
			Logic->HandleBallContact(*Character);
			return false;
		case 2:
			World->GetTimerManager().Tick(4.0f);
			TestEqual(TEXT("Another contact refreshes the boost duration"), Character->GetCharacterMovement()->MaxWalkSpeed, 500.0f);
			TestEqual(TEXT("The unrefreshed stacked boost expires"), StackingCharacter->GetCharacterMovement()->MaxWalkSpeed, 250.0f);
			return false;
		default:
			World->GetTimerManager().Tick(1.1f);
			TestEqual(TEXT("The refreshed boost restores the configured base speed"), Character->GetCharacterMovement()->MaxWalkSpeed, 250.0f);
			World->DestroyWorld(false);
			return true;
		}
	}));
	return true;
}

#endif
