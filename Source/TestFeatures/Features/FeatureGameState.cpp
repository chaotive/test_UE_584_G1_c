#include "FeatureGameState.h"
#include "FeatureSaveGameSubsystem.h"
#include "Features_NonUE/FeatureSaveGameStorage.h"
#include "Engine/GameInstance.h"
#include "TestFeatures.h"

void AFeatureGameState::RecordBounce()
{
	++TotalBounces;
	OnTotalBouncesChanged.Broadcast(TotalBounces);
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameState [%s]: Total bounces: %d"), *GetName(), TotalBounces);
}

int32 AFeatureGameState::GetTotalBounces() const
{
	return TotalBounces;
}

void AFeatureGameState::InitializeTotal(int32 Count)
{
	TotalBounces = Count;
	bSessionInitialized = true;
	OnTotalBouncesChanged.Broadcast(TotalBounces);
}

void AFeatureGameState::StartSession(bool bContinue)
{
	if (bContinue)
	{
		GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>()->RequestLoad();
	}
	else
	{
		InitializeTotal(0);
	}
}

void AFeatureGameState::SaveSession(bool bQuitAfter)
{
	GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>()->RequestSave(TotalBounces, bQuitAfter);
}

void AFeatureGameState::BeginPlay()
{
	Super::BeginPlay();
	UFeatureSaveGameSubsystem* Saves = GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>();
	Saves->OnLoaded.AddUObject(this, &AFeatureGameState::InitializeTotal);
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameState [%s]: BeginPlay (Total bounces: %d)"), *GetName(), TotalBounces);
}

void AFeatureGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UFeatureSaveGameSubsystem* Saves = GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>();
	Saves->OnLoaded.RemoveAll(this);
	Saves->CancelOperation();
	// Editor Stop/forced teardown cannot be deferred for a live HUD; retain a synchronous fallback.
	if (bSessionInitialized && !Saves->HasSavedForQuit())
	{
		FFeatureSaveGameStorage::SaveTotalBounces(TotalBounces);
	}
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameState [%s]: EndPlay (Total bounces: %d)"), *GetName(), TotalBounces);
	Super::EndPlay(EndPlayReason);
}
