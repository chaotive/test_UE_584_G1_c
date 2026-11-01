#include "TrainingDebugHUD.h"
#include "TrainingGameState.h"
#include "Engine/World.h"
#include "training_UE_584_G2_c.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

void UTrainingDebugHUD::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(false);
	BuildLayout();
	UE_LOG(Logtraining_UE_584_G2_c, Log, TEXT("TrainingDebugHUD: Initialized"));
}

void UTrainingDebugHUD::NativeConstruct()
{
	Super::NativeConstruct();
	ATrainingGameState* State = GetWorld()->GetGameState<ATrainingGameState>();
	ObservedState = State;
	State->OnTotalBouncesChanged.AddUObject(this, &UTrainingDebugHUD::SetTotalBounces);
	SetTotalBounces(State->GetTotalBounces());
}

void UTrainingDebugHUD::NativeDestruct()
{
	if (ATrainingGameState* State = ObservedState.Get())
	{
		State->OnTotalBouncesChanged.RemoveAll(this);
	}
	ObservedState.Reset();
	Super::NativeDestruct();
}

void UTrainingDebugHUD::BuildLayout()
{
	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("DebugCanvas"));
	WidgetTree->RootWidget = Canvas;

	UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(BuildPanel());
	PanelSlot->SetPosition(FVector2D(20.0f, 20.0f));
	PanelSlot->SetSize(FVector2D(320.0f, 110.0f));
}

UBorder* UTrainingDebugHUD::BuildPanel()
{
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DebugPanel"));
	Panel->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.85f));
	Panel->SetPadding(FMargin(16.0f, 12.0f));

	UVerticalBox* Rows = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DebugRows"));
	Panel->SetContent(Rows);

	UTextBlock* Title = CreateText(FText::FromString(TEXT("TRAINING DEBUG HUD")), 18);
	Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.85f, 1.0f)));
	Rows->AddChildToVerticalBox(Title)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	Rows->AddChildToVerticalBox(CreateText(FText::FromString(TEXT("C++ feature sandbox")), 14));
	TotalBouncesText = CreateText(FText::FromString(TEXT("Total bounces: 0")), 14);
	Rows->AddChildToVerticalBox(TotalBouncesText);
	return Panel;
}

UTextBlock* UTrainingDebugHUD::CreateText(const FText& Text, int32 FontSize)
{
	UTextBlock* Row = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Row->SetText(Text);
	Row->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
	Row->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	return Row;
}

void UTrainingDebugHUD::SetTotalBounces(int32 TotalBounces)
{
	TotalBouncesText->SetText(FText::Format(NSLOCTEXT("TrainingDebugHUD", "TotalBounces", "Total bounces: {0}"), FText::AsNumber(TotalBounces)));
}
