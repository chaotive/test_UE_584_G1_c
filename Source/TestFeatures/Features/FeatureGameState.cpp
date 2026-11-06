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

void AFeatureGameState::ApplyLoadedTotal(int32 Count)
{
	TotalBounces = Count;
	bHasRestoredTotal = true;
	OnTotalBouncesChanged.Broadcast(TotalBounces);
}

void AFeatureGameState::SaveSession(bool bQuitAfter)
{
	GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>()->RequestSave(TotalBounces, bQuitAfter);
}

void AFeatureGameState::BeginPlay()
{
	Super::BeginPlay();
	UFeatureSaveGameSubsystem* Saves = GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>();
	Saves->OnLoaded.AddUObject(this, &AFeatureGameState::ApplyLoadedTotal);
	Saves->RequestLoad();
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameState [%s]: BeginPlay (Total bounces: %d)"), *GetName(), TotalBounces);
}

void AFeatureGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UFeatureSaveGameSubsystem* Saves = GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>();
	Saves->OnLoaded.RemoveAll(this);
	Saves->CancelOperation();
	// Editor Stop/forced teardown cannot be deferred for a live HUD; retain a synchronous fallback.
	if (bHasRestoredTotal && !Saves->HasSavedForQuit())
	{
		FFeatureSaveGameStorage::SaveTotalBounces(TotalBounces);
	}
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureGameState [%s]: EndPlay (Total bounces: %d)"), *GetName(), TotalBounces);
	Super::EndPlay(EndPlayReason);
}
