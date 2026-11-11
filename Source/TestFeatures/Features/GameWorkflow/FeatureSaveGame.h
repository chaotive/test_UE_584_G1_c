#pragma once

#include "CoreMinimal.h"
#include "Features/Gameplay/FeatureGameplaySessionResult.h"
#include "GameFramework/SaveGame.h"
#include "FeatureSaveGame.generated.h"

// Records the statistics and result of one started session.
// SaveGame marks persistent fields. VisibleAnywhere and BlueprintReadOnly let tools inspect them without editing.
USTRUCT(BlueprintType)
struct TESTFEATURES_API FFeatureGameplaySessionRecord
{
	GENERATED_BODY()

	// Records when this session initialized its gameplay state.
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly)
	FDateTime StartedAtUtc;

	// Counts ball bounces accepted during this session.
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly)
	int32 TotalBounces = 0;

	// Counts blocking ball contacts with player-controlled pawns during this session.
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly)
	int32 PlayerBallCollisions = 0;

	// Records whether this session is running, won, or lost.
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly)
	EFeatureGameplaySessionResult Result = EFeatureGameplaySessionResult::InProgress;
};

// Holds a complete save snapshot.
// SaveGame marks persistent fields. VisibleAnywhere and BlueprintReadOnly keep editor and Blueprint access read-only.
USTRUCT(BlueprintType)
struct TESTFEATURES_API FFeatureGameProgress
{
	GENERATED_BODY()

	// Retains the level to start next, advancing only after victory.
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentLevel = 1;

	// Accumulates bounces across this saved game's sessions.
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly)
	int32 TotalBounces = 0;

	// Retains each started session in chronological order.
	// TArray owns growable value records without retaining world actors.
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly)
	TArray<FFeatureGameplaySessionRecord> Sessions;
};

/** Serialized progress independent of actors and widgets. */
UCLASS()
class TESTFEATURES_API UFeatureSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	// Preserves the aggregate total and all session records.
	// SaveGame marks the progress snapshot for persistence.
	UPROPERTY(SaveGame)
	FFeatureGameProgress Progress;
};
