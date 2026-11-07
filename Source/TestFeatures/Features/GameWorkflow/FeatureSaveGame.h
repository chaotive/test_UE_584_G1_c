#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FeatureSaveGame.generated.h"

/** Value data recorded for one started gameplay session. */
USTRUCT(BlueprintType)
struct TESTFEATURES_API FFeatureGameplaySessionRecord
{
	GENERATED_BODY()

	// Records when this session initialized its gameplay state.
	// FDateTime stores a UTC timestamp by value without object ownership.
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly)
	FDateTime StartedAtUtc;

	// Counts ball bounces accepted during this session.
	// int32 stores the count by value without pointer or reference ownership.
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly)
	int32 TotalBounces = 0;

	// Counts blocking ball contacts with player-controlled pawns during this session.
	// int32 stores the count by value without retaining players or balls.
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly)
	int32 PlayerBallCollisions = 0;
};

/** Complete progress snapshot for one saved game. */
USTRUCT(BlueprintType)
struct TESTFEATURES_API FFeatureGameProgress
{
	GENERATED_BODY()

	// Accumulates bounces across this saved game's sessions.
	// int32 stores the total by value without object ownership.
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
	// The reflected struct stores a value snapshot; SaveGame marks its persistent fields.
	UPROPERTY(SaveGame)
	FFeatureGameProgress Progress;
};
