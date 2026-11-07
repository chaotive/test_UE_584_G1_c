#include "FeatureSaveGamePanel.h"
#include "Features/Gameplay/FeatureGameState.h"
#include "Features/GameWorkflow/FeatureSaveGameSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

void UFeatureSaveGamePanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildLayout();
}

void UFeatureSaveGamePanel::NativeConstruct()
{
	Super::NativeConstruct();
	ObservedState = GetWorld()->GetGameState<AFeatureGameState>();
	UFeatureSaveGameSubsystem* Saves = GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>();
	ObservedSaves = Saves;
	Saves->OnStatusChanged.AddUObject(this, &UFeatureSaveGamePanel::RefreshSaveStatus);
	RefreshSaveStatus(FString(), Saves->IsBusy());
	GetGameInstance()->GetGameViewportClient()->OnWindowCloseRequested().BindUObject(this, &UFeatureSaveGamePanel::HandleWindowCloseRequested);
}

void UFeatureSaveGamePanel::NativeDestruct()
{
	FTSTicker::GetCoreTicker().RemoveTicker(PreviewTicker);
	PreviewTicker.Reset();
	if (UGameViewportClient* Viewport = GetGameInstance()->GetGameViewportClient())
	{
		if (Viewport->OnWindowCloseRequested().IsBoundToObject(this)) { Viewport->OnWindowCloseRequested().Unbind(); }
	}
	if (UFeatureSaveGameSubsystem* Saves = ObservedSaves.Get()) { Saves->OnStatusChanged.RemoveAll(this); }
	ObservedSaves.Reset();
	ObservedState.Reset();
	Super::NativeDestruct();
}

void UFeatureSaveGamePanel::BuildLayout()
{
	UVerticalBox* Rows = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	WidgetTree->RootWidget = Rows;
	Rows->AddChildToVerticalBox(BuildActions());
	SaveDiagnosticsText = CreateText(FText::GetEmpty(), 12);
	SaveDiagnosticsText->SetAutoWrapText(true);
	UScrollBox* Diagnostics = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
	Diagnostics->AddChild(SaveDiagnosticsText);
	Rows->AddChildToVerticalBox(Diagnostics)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
}

UHorizontalBox* UFeatureSaveGamePanel::BuildActions()
{
	UHorizontalBox* Actions = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UButton* SaveButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	SaveButton->SetContent(CreateText(FText::FromString(TEXT("Save game")), 14));
	SaveButton->OnClicked.AddDynamic(this, &UFeatureSaveGamePanel::HandleSaveClicked);
	Actions->AddChildToHorizontalBox(SaveButton);
	UButton* QuitButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	QuitButton->SetContent(CreateText(FText::FromString(TEXT("Save and quit")), 14));
	QuitButton->OnClicked.AddDynamic(this, &UFeatureSaveGamePanel::HandleSaveAndQuitClicked);
	Actions->AddChildToHorizontalBox(QuitButton);
	return Actions;
}

UTextBlock* UFeatureSaveGamePanel::CreateText(const FText& Text, int32 FontSize)
{
	UTextBlock* Row = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Row->SetText(Text);
	Row->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
	Row->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	return Row;
}

FString UFeatureSaveGamePanel::FormatSaveDiagnostics(const UFeatureSaveGameSubsystem& Saves)
{
	const TArray<FString>& History = Saves.GetStatusHistory();
	const FString CurrentStatus = History.IsEmpty() ? TEXT("Idle") : History.Last();
	return FString::Printf(TEXT("I/O state: %s\nSave slot: %s (user 0)\nFile: %s\nI/O history: %s"),
		*CurrentStatus, *Saves.GetSlotName(), *Saves.GetSaveFilePath(), *FString::Join(History, TEXT(" -> ")));
}

void UFeatureSaveGamePanel::RefreshSaveStatus(const FString& Status, bool bBusy)
{
	SaveDiagnosticsText->SetText(FText::FromString(FormatSaveDiagnostics(*ObservedSaves.Get())));
	UFeatureSaveGameSubsystem* Saves = ObservedSaves.Get();
	if (bBusy && (Saves->IsWaitingForIO() || Saves->IsIOComplete()) && !PreviewTicker.IsValid())
	{
		PreviewTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UFeatureSaveGamePanel::HandlePreviewElapsed), 1.0f);
	}
}

bool UFeatureSaveGamePanel::HandlePreviewElapsed(float DeltaSeconds)
{
	PreviewTicker.Reset();
	if (UFeatureSaveGameSubsystem* Saves = ObservedSaves.Get())
	{
		if (Saves->IsWaitingForIO()) { Saves->BeginPendingIO(); }
		else if (Saves->IsIOComplete()) { Saves->FinishOperation(); }
	}
	return false;
}

void UFeatureSaveGamePanel::HandleSaveClicked()
{
	ObservedState->SaveSession(false);
}

void UFeatureSaveGamePanel::HandleSaveAndQuitClicked()
{
	ObservedState->SaveSession(true);
}

bool UFeatureSaveGamePanel::HandleWindowCloseRequested()
{
	HandleSaveAndQuitClicked();
	return false;
}
