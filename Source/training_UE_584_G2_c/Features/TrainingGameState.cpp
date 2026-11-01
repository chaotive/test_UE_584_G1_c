#include "TrainingGameState.h"
#include "TrainingSaveGameSubsystem.h"
#include "Features_NonUE/TrainingSaveGameStorage.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "training_UE_584_G2_c.h"

void ATrainingGameState::RecordBounce()
{
	++TotalBounces;
	OnTotalBouncesChanged.Broadcast(TotalBounces);
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingGameState [%s]: Total bounces: %d"), *GetName(), TotalBounces);
}

int32 ATrainingGameState::GetTotalBounces() const
{
	return TotalBounces;
}

void ATrainingGameState::ApplyLoadedTotal(int32 Count)
{
	TotalBounces = Count;
	bHasRestoredTotal = true;
	OnTotalBouncesChanged.Broadcast(TotalBounces);
}

void ATrainingGameState::SaveSession(bool bQuitAfter)
{
	GetGameInstance()->GetSubsystem<UTrainingSaveGameSubsystem>()->RequestSave(TotalBounces, bQuitAfter);
}

void ATrainingGameState::BeginPlay()
{
	Super::BeginPlay();
	UTrainingSaveGameSubsystem* Saves = GetGameInstance()->GetSubsystem<UTrainingSaveGameSubsystem>();
	Saves->OnLoaded.AddUObject(this, &ATrainingGameState::ApplyLoadedTotal);
	Saves->RequestLoad();
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingGameState [%s]: BeginPlay (Total bounces: %d)"), *GetName(), TotalBounces);
}

void ATrainingGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UTrainingSaveGameSubsystem* Saves = GetGameInstance()->GetSubsystem<UTrainingSaveGameSubsystem>();
	Saves->OnLoaded.RemoveAll(this);
	Saves->CancelOperation();
	// Editor Stop/forced teardown cannot be deferred for a live HUD; retain a synchronous fallback.
	if (bHasRestoredTotal && !Saves->HasSavedForQuit())
	{
		FTrainingSaveGameStorage::SaveTotalBounces(TotalBounces);
	}
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingGameState [%s]: EndPlay (Total bounces: %d)"), *GetName(), TotalBounces);
	Super::EndPlay(EndPlayReason);
}
