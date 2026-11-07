#include "FeatureDebugHUD.h"
#include "Features/UI/Widgets/FeatureSaveGamePanel.h"
#include "Features/UI/Widgets/FeatureBallPanel.h"
#include "GameFramework/PlayerController.h"
#include "TestFeatures.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

void UFeatureDebugHUD::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(false);
	BuildLayout();
	UE_LOG(LogTestFeatures, Log, TEXT("FeatureDebugHUD: Initialized"));
}

void UFeatureDebugHUD::NativeConstruct()
{
	Super::NativeConstruct();
	GetOwningPlayer()->bShowMouseCursor = true;
	GetOwningPlayer()->SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));
}

void UFeatureDebugHUD::BuildLayout()
{
	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("DebugCanvas"));
	WidgetTree->RootWidget = Canvas;

	UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(BuildPanel());
	PanelSlot->SetPosition(FVector2D(20.0f, 20.0f));
	PanelSlot->SetSize(FVector2D(700.0f, 290.0f));
}

UBorder* UFeatureDebugHUD::BuildPanel()
{
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DebugPanel"));
	Panel->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.85f));
	Panel->SetPadding(FMargin(16.0f, 12.0f));

	UVerticalBox* Rows = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DebugRows"));
	Panel->SetContent(Rows);

	UTextBlock* Title = CreateText(FText::FromString(TEXT("TRAINING DEBUG HUD")), 18);
	Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.85f, 1.0f)));
	Rows->AddChildToVerticalBox(Title)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	Rows->AddChildToVerticalBox(CreateText(FText::FromString(TEXT("Crazy balls")), 14));
	BallPanel = CreateWidget<UFeatureBallPanel>(GetOwningPlayer(), UFeatureBallPanel::StaticClass());
	Rows->AddChildToVerticalBox(BallPanel);
	SavePanel = CreateWidget<UFeatureSaveGamePanel>(GetOwningPlayer(), UFeatureSaveGamePanel::StaticClass());
	Rows->AddChildToVerticalBox(SavePanel)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	return Panel;
}

UTextBlock* UFeatureDebugHUD::CreateText(const FText& Text, int32 FontSize)
{
	UTextBlock* Row = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Row->SetText(Text);
	Row->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
	Row->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	return Row;
}
