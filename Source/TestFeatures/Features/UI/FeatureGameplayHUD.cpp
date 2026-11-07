#include "FeatureGameplayHUD.h"
#include "Features/Gameplay/FeatureGameState.h"
#include "Features/GameWorkflow/FeatureGameplaySession.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/World.h"
#include "Styling/CoreStyle.h"

void UFeatureGameplayHUD::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildLayout();
}

void UFeatureGameplayHUD::NativeConstruct()
{
	Super::NativeConstruct();
	AFeatureGameState* State = GetWorld()->GetGameState<AFeatureGameState>();
	ObservedState = State;
	ObservedSession = GetWorld()->GetSubsystem<UFeatureGameplaySession>();
	State->OnSessionStatsChanged.AddUObject(this, &UFeatureGameplayHUD::UpdateStats);
	const TArray<FFeatureGameplaySessionRecord>& Sessions = State->GetGameplaySessions();
	UpdateStats(Sessions.IsEmpty() ? FFeatureGameplaySessionRecord() : Sessions.Last());
}

void UFeatureGameplayHUD::NativeDestruct()
{
	if (AFeatureGameState* State = ObservedState.Get())
	{
		State->OnSessionStatsChanged.RemoveAll(this);
	}
	ObservedState.Reset();
	ObservedSession.Reset();
	Super::NativeDestruct();
}

void UFeatureGameplayHUD::BuildLayout()
{
	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
	WidgetTree->RootWidget = Canvas;
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>();
	Panel->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.85f));
	Panel->SetPadding(FMargin(16.0f, 12.0f));
	UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(Panel);
	PanelSlot->SetAnchors(FAnchors(1.0f, 0.0f));
	PanelSlot->SetAlignment(FVector2D(1.0f, 0.0f));
	PanelSlot->SetPosition(FVector2D(-20.0f, 20.0f));
	PanelSlot->SetAutoSize(true);

	UVerticalBox* Rows = WidgetTree->ConstructWidget<UVerticalBox>();
	Panel->SetContent(Rows);
	PlayerContactsText = WidgetTree->ConstructWidget<UTextBlock>();
	BallBouncesText = WidgetTree->ConstructWidget<UTextBlock>();
	PlayerContactsText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 18));
	BallBouncesText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 18));
	PlayerContactsText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	BallBouncesText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	Rows->AddChildToVerticalBox(PlayerContactsText);
	Rows->AddChildToVerticalBox(BallBouncesText);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UFeatureGameplayHUD::UpdateStats(const FFeatureGameplaySessionRecord& Stats)
{
	PlayerContactsText->SetText(FText::Format(NSLOCTEXT("FeatureGameplayHUD", "WinProgress", "Player contacts: {0} / {1} to win"),
		FText::AsNumber(Stats.PlayerBallCollisions), FText::AsNumber(ObservedSession->GetPlayerContactsToWin())));
	BallBouncesText->SetText(FText::Format(NSLOCTEXT("FeatureGameplayHUD", "LossProgress", "Ball bounces: {0} / {1} to lose"),
		FText::AsNumber(Stats.TotalBounces), FText::AsNumber(ObservedSession->GetSessionBouncesToLose())));
}
