#if WITH_DEV_AUTOMATION_TESTS

#include "test_UE_584_G1_cCharacter.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "TimerManager.h"

// Checks the configured base speed, temporary contact boost, and timed restoration.
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
	Character->ConfigureFeatureSpeed(0.5f, 2.0f, 5.0f, false);
	TestEqual(TEXT("Base multiplier configures the baseline walk speed"), Character->GetCharacterMovement()->MaxWalkSpeed, 250.0f);
	Character->HandleFeatureBallContact(*Character);
	TestEqual(TEXT("Ball contact temporarily doubles the base speed"), Character->GetCharacterMovement()->MaxWalkSpeed, 500.0f);
	World->GetTimerManager().Tick(5.1f);
	TestEqual(TEXT("The timer restores the configured base speed"), Character->GetCharacterMovement()->MaxWalkSpeed, 250.0f);

	Character->ConfigureFeatureSpeed(0.5f, 2.0f, 5.0f, true);
	Character->HandleFeatureBallContact(*Character);
	Character->HandleFeatureBallContact(*Character);
	TestEqual(TEXT("Stacking adds one contact boost per contact"), Character->GetCharacterMovement()->MaxWalkSpeed, 750.0f);

	World->DestroyWorld(false);
	return true;
}

#endif
