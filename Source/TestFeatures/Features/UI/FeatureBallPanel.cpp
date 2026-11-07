#include "FeatureBallPanel.h"
#include "Features/Gameplay/FeatureGameState.h"
#include "Engine/World.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Styling/CoreStyle.h"

void UFeatureBallPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildLayout();
}

void UFeatureBallPanel::NativeConstruct()
{
	Super::NativeConstruct();
	AFeatureGameState* State = GetWorld()->GetGameState<AFeatureGameState>();
	ObservedState = State;
	State->OnTotalBouncesChanged.AddUObject(this, &UFeatureBallPanel::SetTotalBounces);
	State->OnCurrentLevelChanged.AddUObject(this, &UFeatureBallPanel::SetCurrentLevel);
	SetTotalBounces(State->GetTotalBounces());
	SetCurrentLevel(State->GetCurrentLevel());
}

void UFeatureBallPanel::NativeDestruct()
{
	if (AFeatureGameState* State = ObservedState.Get())
	{
		State->OnTotalBouncesChanged.RemoveAll(this);
		State->OnCurrentLevelChanged.RemoveAll(this);
	}
	ObservedState.Reset();
	Super::NativeDestruct();
}

void UFeatureBallPanel::BuildLayout()
{
	UVerticalBox* Rows = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BallRows"));
	WidgetTree->RootWidget = Rows;
	CurrentLevelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CurrentLevel"));
	TotalBouncesText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TotalBounces"));
	CurrentLevelText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 14));
	TotalBouncesText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 14));
	CurrentLevelText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	TotalBouncesText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	Rows->AddChildToVerticalBox(CurrentLevelText);
	Rows->AddChildToVerticalBox(TotalBouncesText);
	SetCurrentLevel(1);
	SetTotalBounces(0);
}

void UFeatureBallPanel::SetTotalBounces(int32 TotalBounces)
{
	TotalBouncesText->SetText(FText::Format(NSLOCTEXT("FeatureDebugHUD", "TotalBounces", "Total bounces: {0}"), FText::AsNumber(TotalBounces)));
}

void UFeatureBallPanel::SetCurrentLevel(int32 CurrentLevel)
{
	CurrentLevelText->SetText(FText::Format(NSLOCTEXT("FeatureDebugHUD", "CurrentLevel", "Current level: {0}"),
		FText::AsNumber(CurrentLevel)));
}
