#if WITH_DEV_AUTOMATION_TESTS

#include "test_UE_584_G1_cCharacter.h"
#include "Containers/Array.h"
#include "Containers/Ticker.h"
#include "Engine/World.h"
#include "Features_GameLogic/FeaturePlayerRules.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Templates/Function.h"
#include "TimerManager.h"

// Defines player-speed scenarios against the project's third-person character.
// Unreal registers this spec under the Feature.Smoke filter.
BEGIN_DEFINE_SPEC(FFeaturePlayerSpeedBoostSpec, "Feature.Smoke.PlayerSpeedBoost",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
	// Spawns a character with the scenario's boost configuration.
	// Returns a borrowed reference to an actor owned by the temporary world.
	Atest_UE_584_G1_cCharacter& SpawnPlayer(bool bStackBonuses);

	// Hosts the players and timers for one scenario.
	// The world is rooted by Unreal and destroyed in AfterEach.
	UWorld* World = nullptr;

	// Selects the Blueprint that supplies the project's character defaults.
	// The loaded class is borrowed from Unreal for the duration of the fixture.
	UClass* CharacterClass = nullptr;

	// Tracks the scheduled timer steps until the scenario finishes or times out.
	// The ticker handle allows AfterEach to cancel callbacks before destroying the world.
	FTSTicker::FDelegateHandle TimerSteps;
END_DEFINE_SPEC(FFeaturePlayerSpeedBoostSpec)

void FFeaturePlayerSpeedBoostSpec::Define()
{
	BeforeEach([this]()
	{
		World = UWorld::CreateWorld(EWorldType::Game, false);
		CharacterClass = LoadClass<Atest_UE_584_G1_cCharacter>(nullptr,
			TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C"));
		TestNotNull("Third-person character Blueprint class loads", CharacterClass);
	});

	It("applies a temporary speed boost on contact", [this]()
	{
		Atest_UE_584_G1_cCharacter& Player = SpawnPlayer(false);
		UFeaturePlayerRules& Rules = *Player.FindComponentByClass<UFeaturePlayerRules>();
		const UCharacterMovementComponent& Movement = *Player.GetCharacterMovement();
		TestEqual("Configured base speed", Movement.MaxWalkSpeed, 250.0f);

		Rules.HandleBallContact(Player);

		TestEqual("Boosted speed", Movement.MaxWalkSpeed, 500.0f);
	});

	It("stacks its own contacts and ignores contacts for another pawn", [this]()
	{
		Atest_UE_584_G1_cCharacter& OtherPlayer = SpawnPlayer(false);
		Atest_UE_584_G1_cCharacter& Player = SpawnPlayer(true);
		UFeaturePlayerRules& Rules = *Player.FindComponentByClass<UFeaturePlayerRules>();
		const UCharacterMovementComponent& Movement = *Player.GetCharacterMovement();

		Rules.HandleBallContact(OtherPlayer);
		TestEqual("Speed after another pawn's contact", Movement.MaxWalkSpeed, 250.0f);

		Rules.HandleBallContact(Player);
		Rules.HandleBallContact(Player);
		TestEqual("Speed after two contacts", Movement.MaxWalkSpeed, 750.0f);
	});

	LatentIt("refreshes on another contact and expires after the refreshed duration", [this](const FDoneDelegate& Done)
	{
		Atest_UE_584_G1_cCharacter& Refreshed = SpawnPlayer(false);
		UFeaturePlayerRules& RefreshedRules = *Refreshed.FindComponentByClass<UFeaturePlayerRules>();
		RefreshedRules.HandleBallContact(Refreshed);

		Atest_UE_584_G1_cCharacter& Unrefreshed = SpawnPlayer(true);
		UFeaturePlayerRules& UnrefreshedRules = *Unrefreshed.FindComponentByClass<UFeaturePlayerRules>();
		UnrefreshedRules.HandleBallContact(Unrefreshed);
		UnrefreshedRules.HandleBallContact(Unrefreshed);

		TArray<TFunction<void()>> Steps;
		// Each step runs on a separate engine frame so the timer manager can advance.
		Steps.Emplace([this]()
		{
			World->GetTimerManager().Tick(0.0f);
		});
		Steps.Emplace([this, &Refreshed, &RefreshedRules]()
		{
			World->GetTimerManager().Tick(4.0f);
			TestEqual("The boost is active before expiry", Refreshed.GetCharacterMovement()->MaxWalkSpeed, 500.0f);
			RefreshedRules.HandleBallContact(Refreshed);
		});
		Steps.Emplace([this, &Refreshed, &Unrefreshed]()
		{
			World->GetTimerManager().Tick(4.0f);
			TestEqual("The refreshed boost remains active", Refreshed.GetCharacterMovement()->MaxWalkSpeed, 500.0f);
			TestEqual("The unrefreshed stacked boost expires", Unrefreshed.GetCharacterMovement()->MaxWalkSpeed, 250.0f);
		});
		Steps.Emplace([this, &Refreshed]()
		{
			World->GetTimerManager().Tick(1.1f);
			TestEqual("Expiry restores the configured base speed", Refreshed.GetCharacterMovement()->MaxWalkSpeed, 250.0f);
		});

		TimerSteps = FTSTicker::GetCoreTicker().AddTicker(TEXT("FeatureSpeedBoostSpec"), 0.0f,
			[Steps = MoveTemp(Steps), NextStep = 0, Done](float) mutable
			{
				Steps[NextStep++]();
				if (NextStep < Steps.Num())
				{
					return true;
				}
				Done.Execute();
				return false;
			});
	});

	AfterEach([this]()
	{
		FTSTicker::RemoveTicker(TimerSteps);
		TimerSteps.Reset();
		World->DestroyWorld(false);
		World = nullptr;
		CharacterClass = nullptr;
	});
}

Atest_UE_584_G1_cCharacter& FFeaturePlayerSpeedBoostSpec::SpawnPlayer(bool bStackBonuses)
{
	Atest_UE_584_G1_cCharacter& Player = *World->SpawnActor<Atest_UE_584_G1_cCharacter>(CharacterClass);
	Player.FindComponentByClass<UFeaturePlayerRules>()->ConfigureSpeed(0.5f, 2.0f, 5.0f, bStackBonuses);
	return Player;
}

#endif
