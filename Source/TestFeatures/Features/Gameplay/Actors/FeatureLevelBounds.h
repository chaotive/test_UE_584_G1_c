#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FeatureLevelBounds.generated.h"

class UBoxComponent;

/** Six collision faces that contain physics balls inside the level. */
UCLASS()
class TESTFEATURES_API AFeatureLevelBounds : public AActor
{
	GENERATED_BODY()

public:
	// Creates the enclosure's root and six collision faces.
	// No pointer or reference parameters; the world owns this actor and its components.
	AFeatureLevelBounds();

	// Resizes the enclosure's interior before the session launches its balls.
	// NewHalfExtent is a borrowed read-only FVector reference with dimensions in centimeters.
	void SetHalfExtent(const FVector& NewHalfExtent);

protected:
	// Updates collision geometry when the actor is constructed or edited.
	// Transform is a borrowed read-only reference to the actor's world transform.
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	// Defines the clear interior distance from the actor origin to each face.
	// FVector is stored by value; editor metadata exposes positive dimensions in centimeters.
	UPROPERTY(EditAnywhere, Category="Feature Bounds", meta=(ClampMin="1"))
	FVector HalfExtent = FVector(6000.0f, 6000.0f, 3000.0f);

	// Gives each wall enough depth to provide a solid collision surface.
	// float is stored by value with positive centimeter units exposed in the editor.
	UPROPERTY(EditAnywhere, Category="Feature Bounds", meta=(ClampMin="1"))
	float WallThickness = 100.0f;

	// Retains the six collision components in positive and negative axis pairs.
	// UPROPERTY tracks strong TObjectPtr entries in a TArray; this actor owns the components.
	UPROPERTY(VisibleAnywhere, Category="Feature Bounds")
	TArray<TObjectPtr<UBoxComponent>> Walls;

	// Places and sizes the six faces around the clear interior.
	// No pointer or reference parameters or return value; updates actor-owned components.
	void UpdateWalls();
};
