#include "TrainingBallPanel.h"
#include "TrainingGameState.h"
#include "Engine/World.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Styling/CoreStyle.h"

void UTrainingBallPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildLayout();
}

void UTrainingBallPanel::NativeConstruct()
{
	Super::NativeConstruct();
	ATrainingGameState* State = GetWorld()->GetGameState<ATrainingGameState>();
	ObservedState = State;
	State->OnTotalBouncesChanged.AddUObject(this, &UTrainingBallPanel::SetTotalBounces);
	SetTotalBounces(State->GetTotalBounces());
}

void UTrainingBallPanel::NativeDestruct()
{
	if (ATrainingGameState* State = ObservedState.Get())
	{
		State->OnTotalBouncesChanged.RemoveAll(this);
	}
	ObservedState.Reset();
	Super::NativeDestruct();
}

void UTrainingBallPanel::BuildLayout()
{
	TotalBouncesText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TotalBounces"));
	TotalBouncesText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 14));
	TotalBouncesText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	WidgetTree->RootWidget = TotalBouncesText;
	SetTotalBounces(0);
}

void UTrainingBallPanel::SetTotalBounces(int32 TotalBounces)
{
	TotalBouncesText->SetText(FText::Format(NSLOCTEXT("TrainingDebugHUD", "TotalBounces", "Total bounces: {0}"), FText::AsNumber(TotalBounces)));
}
