#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FeatureProgressPersistence.generated.h"

class AFeatureGameState;
class UFeatureSaveGameSubsystem;

/** Binds a world's progress state to persistence operations and actor teardown. */
UCLASS()
class TESTFEATURES_API UFeatureProgressPersistence : public UActorComponent
{
	GENERATED_BODY()

public:
	// Creates an event-driven persistence binding without a per-frame tick.
	// The owning actor retains this component.
	UFeatureProgressPersistence();

	// Attaches the persistence workflow when GameMode provisions its game state.
	// State is borrowed; it owns the created component through its instance-component collection.
	static void AttachTo(AFeatureGameState& State);

	// Loads continued progress or initializes a fresh session from configuration.
	void StartSession(bool bContinue, int32 InitialLevel);

	// Requests a snapshot save and optionally an orderly quit after success.
	// The persistence subsystem copies the borrowed progress snapshot.
	UFUNCTION(BlueprintCallable, Category="Feature Progress")
	void SaveSession(bool bQuitAfter = false);

protected:
	// Connects loaded snapshots to the state once its actor enters gameplay.
	// Observes the owner and game-instance subsystem weakly.
	virtual void BeginPlay() override;

	// Unsubscribes, cancels stale operations, and performs the teardown save fallback.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Observes the state that supplies and receives progress values.
	// TWeakObjectPtr does not retain the owning world actor.
	TWeakObjectPtr<AFeatureGameState> GameState;

	// Observes the asynchronous operation coordinator across this world's lifetime.
	// TWeakObjectPtr does not retain the game-instance-owned subsystem.
	TWeakObjectPtr<UFeatureSaveGameSubsystem> Saves;

	// Identifies this binding's loaded-snapshot subscription for precise removal.
	// FDelegateHandle is a value token without listener or publisher ownership.
	FDelegateHandle LoadedHandle;
};
