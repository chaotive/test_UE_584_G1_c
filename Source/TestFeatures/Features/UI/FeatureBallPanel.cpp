#include "FeatureBallPanel.h"
#include "Features/Gameplay/FeatureGameState.h"
#include "Engine/World.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
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
	SetTotalBounces(State->GetTotalBounces());
}

void UFeatureBallPanel::NativeDestruct()
{
	if (AFeatureGameState* State = ObservedState.Get())
	{
		State->OnTotalBouncesChanged.RemoveAll(this);
	}
	ObservedState.Reset();
	Super::NativeDestruct();
}

void UFeatureBallPanel::BuildLayout()
{
	TotalBouncesText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TotalBounces"));
	TotalBouncesText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 14));
	TotalBouncesText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	WidgetTree->RootWidget = TotalBouncesText;
	SetTotalBounces(0);
}

void UFeatureBallPanel::SetTotalBounces(int32 TotalBounces)
{
	TotalBouncesText->SetText(FText::Format(NSLOCTEXT("FeatureDebugHUD", "TotalBounces", "Total bounces: {0}"), FText::AsNumber(TotalBounces)));
}
