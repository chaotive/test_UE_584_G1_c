#include "FeatureLevelBounds.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"

AFeatureLevelBounds::AFeatureLevelBounds()
{
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("BoundsRoot")));
	const FName WallNames[] = { TEXT("East"), TEXT("West"), TEXT("North"), TEXT("South"), TEXT("Ceiling"), TEXT("Floor") };
	for (const FName WallName : WallNames)
	{
		UBoxComponent* Wall = CreateDefaultSubobject<UBoxComponent>(WallName);
		Wall->SetupAttachment(RootComponent);
		Wall->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Wall->SetCollisionObjectType(ECC_WorldStatic);
		Wall->SetCollisionResponseToAllChannels(ECR_Ignore);
		Wall->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
		Walls.Add(Wall);
	}
	UpdateWalls();
}

void AFeatureLevelBounds::SetHalfExtent(const FVector& NewHalfExtent)
{
	HalfExtent = NewHalfExtent;
	UpdateWalls();
}

void AFeatureLevelBounds::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateWalls();
}

void AFeatureLevelBounds::UpdateWalls()
{
	const float HalfThickness = WallThickness * 0.5f;
	const FVector FaceExtents[] = {
		FVector(HalfThickness, HalfExtent.Y + WallThickness, HalfExtent.Z + WallThickness),
		FVector(HalfExtent.X + WallThickness, HalfThickness, HalfExtent.Z + WallThickness),
		FVector(HalfExtent.X + WallThickness, HalfExtent.Y + WallThickness, HalfThickness)
	};
	for (int32 Axis = 0; Axis < 3; ++Axis)
	{
		FVector Offset = FVector::ZeroVector;
		Offset[Axis] = HalfExtent[Axis] + HalfThickness;
		Walls[Axis * 2]->SetRelativeLocation(Offset);
		Walls[Axis * 2 + 1]->SetRelativeLocation(-Offset);
		Walls[Axis * 2]->SetBoxExtent(FaceExtents[Axis]);
		Walls[Axis * 2 + 1]->SetBoxExtent(FaceExtents[Axis]);
	}
}
