#include "FeatureBallManager.h"
#include "Actors/FeatureBall.h"
#include "Features_GameLogic/FeatureBallRules.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "TestFeatures.h"

void UFeatureBallManager::Deinitialize()
{
	Stop();
	for (const TWeakObjectPtr<AFeatureBall>& Entry : SpawnedBalls)
	{
		if (AFeatureBall* Ball = Entry.Get())
		{
			Ball->OnContact.RemoveAll(this);
			Ball->OnRetired.RemoveAll(this);
		}
	}
	Super::Deinitialize();
}

void UFeatureBallManager::Start(int32 BallCount, float InBallSize, float InBallSpeedMultiplier, int32 ReboundLimit, const FBox& SpawnBounds)
{
	bRunning = true;
	BallSize = InBallSize;
	BallSpeedMultiplier = InBallSpeedMultiplier;
	BallReboundLimit = ReboundLimit;
	BallSpawnBounds = SpawnBounds;
	Prewarm(BallCount);
	SpawnMultiple(BallCount, SpawnBounds);
}

void UFeatureBallManager::Pause()
{
	bRunning = false;
}

void UFeatureBallManager::Resume()
{
	bRunning = true;
	while (CanReplaceBalls() && PendingBallReplacements > 0)
	{
		SpawnReplacementBall();
	}
}

void UFeatureBallManager::Stop()
{
	bRunning = false;
	PendingBallReplacements = 0;
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void UFeatureBallManager::HandleBallContact(AFeatureBall&, AActor& OtherActor)
{
	APawn* Pawn = Cast<APawn>(&OtherActor);
	if (Pawn && Pawn->IsPlayerControlled())
	{
		OnPlayerBallContact.Broadcast(*Pawn);
	}
}

void UFeatureBallManager::HandleBallRetired(AFeatureBall& Ball)
{
	if (CanReplaceBalls())
	{
		++PendingBallReplacements;
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UFeatureBallManager::SpawnReplacementBall);
	}
}

void UFeatureBallManager::SpawnReplacementBall()
{
	if (CanReplaceBalls() && PendingBallReplacements > 0)
	{
		--PendingBallReplacements;
		Spawn(FMath::RandPointInBox(BallSpawnBounds));
	}
}

bool UFeatureBallManager::CanReplaceBalls() const
{
	return GetWorld()->GetSubsystem<UFeatureBallRules>()->ShouldReplaceBall(bRunning);
}

void UFeatureBallManager::Prewarm(int32 Count)
{
	while (SpawnedBalls.Num() < Count)
	{
		CreateBall(FVector::ZeroVector);
	}
}

void UFeatureBallManager::SpawnMultiple(int32 Count, const FBox& SpawnBounds)
{
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureBallManager: Spawning %d balls (Min: %s, Max: %s)"),
		Count, *SpawnBounds.Min.ToString(), *SpawnBounds.Max.ToString());

	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FVector Location = FMath::RandPointInBox(SpawnBounds);
		Spawn(Location);
	}
}

AFeatureBall* UFeatureBallManager::Spawn(const FVector& Location)
{
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureBallManager: Spawn started (Location: %s)"), *Location.ToString());

	AFeatureBall* Ball = FindAvailableBall();
	if (Ball)
	{
		UE_LOG(LogTestFeatures, Log, TEXT("FeatureBallManager [%s]: Reusing pooled ball"), *Ball->GetName());
	}
	else
	{
		Ball = CreateBall(Location);
	}

	Ball->Activate(Location);
	return Ball;
}

AFeatureBall* UFeatureBallManager::FindAvailableBall() const
{
	for (const TWeakObjectPtr<AFeatureBall>& Entry : SpawnedBalls)
	{
		AFeatureBall* Ball = Entry.Get();
		if (Ball && !Ball->IsActive())
		{
			return Ball;
		}
	}
	return nullptr;
}

AFeatureBall* UFeatureBallManager::CreateBall(const FVector& Location)
{
	FActorSpawnParameters SpawnParameters;
	const FString BallName = FString::Printf(TEXT("Training_Ball_%d"), SpawnedBalls.Num());
	SpawnParameters.Name = FName(*BallName);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFeatureBall* Ball = GetWorld()->SpawnActor<AFeatureBall>(Location, FRotator::ZeroRotator, SpawnParameters);
#if WITH_EDITOR
	Ball->SetActorLabel(BallName);
#endif
	SpawnedBalls.Add(Ball);
	Ball->Configure(BallSize, BallSpeedMultiplier, BallReboundLimit);
	// Subscribe scoring observers before contact reactions, preserving the existing event order.
	OnBallCreated.Broadcast(*Ball);
	Ball->OnContact.AddUObject(this, &UFeatureBallManager::HandleBallContact);
	Ball->OnRetired.AddUObject(this, &UFeatureBallManager::HandleBallRetired);

	UE_LOG(LogTestFeatures, Log, TEXT("FeatureBallManager [%s]: Pool grew to %d balls"), *Ball->GetName(), SpawnedBalls.Num());
	return Ball;
}

const TArray<TWeakObjectPtr<AFeatureBall>>& UFeatureBallManager::GetSpawnedBalls() const
{
	return SpawnedBalls;
}
