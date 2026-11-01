#include "TrainingGameState.h"
#include "TrainingSaveGameSubsystem.h"
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

void ATrainingGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	// Editor preview worlds must not load session data; only gameplay worlds resume a save.
	if (GetWorld()->IsGameWorld())
	{
		TotalBounces = GetGameInstance()->GetSubsystem<UTrainingSaveGameSubsystem>()->LoadTotalBounces();
		OnTotalBouncesChanged.Broadcast(TotalBounces);
	}
}

void ATrainingGameState::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingGameState [%s]: BeginPlay (Total bounces: %d)"), *GetName(), TotalBounces);
}

void ATrainingGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetGameInstance()->GetSubsystem<UTrainingSaveGameSubsystem>()->SaveTotalBounces(TotalBounces);
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingGameState [%s]: EndPlay (Total bounces: %d)"), *GetName(), TotalBounces);
	Super::EndPlay(EndPlayReason);
}
