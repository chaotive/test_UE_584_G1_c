#include "FeatureProgressPersistence.h"
#include "Features/Gameplay/FeatureGameState.h"
#include "FeatureSaveGame.h"
#include "FeatureSaveGameSubsystem.h"
#include "Features_NonUE/FeatureSaveGameStorage.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UFeatureProgressPersistence::UFeatureProgressPersistence()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFeatureProgressPersistence::AttachTo(AFeatureGameState& State)
{
	UFeatureProgressPersistence* Binding = NewObject<UFeatureProgressPersistence>(&State);
	State.AddInstanceComponent(Binding);
	Binding->RegisterComponent();
}

void UFeatureProgressPersistence::BeginPlay()
{
	Super::BeginPlay();
	GameState = CastChecked<AFeatureGameState>(GetOwner());
	Saves = GetWorld()->GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>();
	LoadedHandle = Saves->OnLoaded.AddUObject(GameState.Get(), &AFeatureGameState::InitializeSession);
}

void UFeatureProgressPersistence::StartSession(bool bContinue, int32 InitialLevel)
{
	if (bContinue)
	{
		Saves->RequestLoad();
	}
	else
	{
		FFeatureGameProgress FreshProgress;
		FreshProgress.CurrentLevel = InitialLevel;
		GameState->InitializeSession(FreshProgress);
	}
}

void UFeatureProgressPersistence::SaveSession(bool bQuitAfter)
{
	Saves->RequestSave(GameState->GetProgress(), bQuitAfter);
}

void UFeatureProgressPersistence::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Saves->OnLoaded.Remove(LoadedHandle);
	Saves->CancelOperation();
	// Editor Stop/forced teardown cannot wait for async I/O or a live HUD.
	if (GameState->IsSessionInitialized() && !Saves->HasSavedForQuit())
	{
		FFeatureSaveGameStorage::SaveProgress(GameState->GetProgress());
	}
	GameState.Reset();
	Saves.Reset();
	Super::EndPlay(EndPlayReason);
}
