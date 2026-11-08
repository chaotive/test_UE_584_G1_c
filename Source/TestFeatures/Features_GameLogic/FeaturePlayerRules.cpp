#include "FeaturePlayerRules.h"
#include "Features/GameWorkflow/FeatureGameplaySession.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include <angelscript.h>

UFeaturePlayerRules::UFeaturePlayerRules()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UFeaturePlayerRules::ConfigureSpeed(float BaseMultiplier, float ContactMultiplier, float Duration, bool bStackBonuses)
{
    Movement = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
    NativeWalkSpeed = Movement->MaxWalkSpeed;
    BaseSpeedMultiplier = BaseMultiplier;
    ContactSpeedMultiplier = ContactMultiplier;
    BoostDuration = Duration;
    bStackSpeedBonuses = bStackBonuses;
    Runtime = MakeUnique<FFeatureScriptRuntime>(TEXT("FeaturePlayerRules.as"));
    BonusRule = &Runtime->FindFunction("int PlayerSpeedBonusCount(int, bool, float)");
    SpeedRule = &Runtime->FindFunction("float PlayerWalkSpeed(float, float, float, int)");
    GetWorld()->GetTimerManager().ClearTimer(SpeedTimer);
    ResetSpeed();
}

void UFeaturePlayerRules::BeginPlay()
{
    Super::BeginPlay();
    Session = GetWorld()->GetSubsystem<UFeatureGameplaySession>();
    Session->OnPlayerBallContact.AddUObject(this, &UFeaturePlayerRules::HandleBallContact);
}

void UFeaturePlayerRules::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Session->OnPlayerBallContact.RemoveAll(this);
    GetWorld()->GetTimerManager().ClearTimer(SpeedTimer);
    Runtime.Reset();
    Super::EndPlay(EndPlayReason);
}

void UFeaturePlayerRules::HandleBallContact(APawn& Pawn)
{
    if (&Pawn != GetOwner())
    {
        return;
    }
    AngelScript::asIScriptContext& Context = Runtime->Prepare(*BonusRule);
    Context.SetArgDWord(0, SpeedBonusCount);
    Context.SetArgByte(1, bStackSpeedBonuses);
    Context.SetArgFloat(2, BoostDuration);
    Runtime->Execute();
    SpeedBonusCount = static_cast<int32>(Context.GetReturnDWord());
    ApplySpeed();
    if (SpeedBonusCount > 0)
    {
        GetWorld()->GetTimerManager().SetTimer(SpeedTimer, this, &UFeaturePlayerRules::ResetSpeed, BoostDuration, false);
    }
}

void UFeaturePlayerRules::ResetSpeed()
{
    SpeedBonusCount = 0;
    ApplySpeed();
}

void UFeaturePlayerRules::ApplySpeed()
{
    AngelScript::asIScriptContext& Context = Runtime->Prepare(*SpeedRule);
    Context.SetArgFloat(0, NativeWalkSpeed);
    Context.SetArgFloat(1, BaseSpeedMultiplier);
    Context.SetArgFloat(2, ContactSpeedMultiplier);
    Context.SetArgDWord(3, SpeedBonusCount);
    Runtime->Execute();
    Movement->MaxWalkSpeed = Context.GetReturnFloat();
}
