#include "TrainingSaveGamePanel.h"
#include "TrainingGameState.h"
#include "TrainingSaveGameSubsystem.h"
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

void UTrainingSaveGamePanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildLayout();
}

void UTrainingSaveGamePanel::NativeConstruct()
{
	Super::NativeConstruct();
	ObservedState = GetWorld()->GetGameState<ATrainingGameState>();
	UTrainingSaveGameSubsystem* Saves = GetGameInstance()->GetSubsystem<UTrainingSaveGameSubsystem>();
	ObservedSaves = Saves;
	Saves->OnStatusChanged.AddUObject(this, &UTrainingSaveGamePanel::RefreshSaveStatus);
	RefreshSaveStatus(FString(), Saves->IsBusy());
	GetGameInstance()->GetGameViewportClient()->OnWindowCloseRequested().BindUObject(this, &UTrainingSaveGamePanel::HandleWindowCloseRequested);
}

void UTrainingSaveGamePanel::NativeDestruct()
{
	FTSTicker::GetCoreTicker().RemoveTicker(PreviewTicker);
	PreviewTicker.Reset();
	if (UGameViewportClient* Viewport = GetGameInstance()->GetGameViewportClient())
	{
		if (Viewport->OnWindowCloseRequested().IsBoundToObject(this)) { Viewport->OnWindowCloseRequested().Unbind(); }
	}
	if (UTrainingSaveGameSubsystem* Saves = ObservedSaves.Get()) { Saves->OnStatusChanged.RemoveAll(this); }
	ObservedSaves.Reset();
	ObservedState.Reset();
	Super::NativeDestruct();
}

void UTrainingSaveGamePanel::BuildLayout()
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

UHorizontalBox* UTrainingSaveGamePanel::BuildActions()
{
	UHorizontalBox* Actions = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UButton* SaveButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	SaveButton->SetContent(CreateText(FText::FromString(TEXT("Save game")), 14));
	SaveButton->OnClicked.AddDynamic(this, &UTrainingSaveGamePanel::HandleSaveClicked);
	Actions->AddChildToHorizontalBox(SaveButton);
	UButton* QuitButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	QuitButton->SetContent(CreateText(FText::FromString(TEXT("Save and quit")), 14));
	QuitButton->OnClicked.AddDynamic(this, &UTrainingSaveGamePanel::HandleSaveAndQuitClicked);
	Actions->AddChildToHorizontalBox(QuitButton);
	return Actions;
}

UTextBlock* UTrainingSaveGamePanel::CreateText(const FText& Text, int32 FontSize)
{
	UTextBlock* Row = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Row->SetText(Text);
	Row->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
	Row->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	return Row;
}

FString UTrainingSaveGamePanel::FormatSaveDiagnostics(const UTrainingSaveGameSubsystem& Saves)
{
	const TArray<FString>& History = Saves.GetStatusHistory();
	const FString CurrentStatus = History.IsEmpty() ? TEXT("Idle") : History.Last();
	return FString::Printf(TEXT("I/O state: %s\nSave slot: %s (user 0)\nFile: %s\nI/O history: %s"),
		*CurrentStatus, *Saves.GetSlotName(), *Saves.GetSaveFilePath(), *FString::Join(History, TEXT(" -> ")));
}

void UTrainingSaveGamePanel::RefreshSaveStatus(const FString& Status, bool bBusy)
{
	SaveDiagnosticsText->SetText(FText::FromString(FormatSaveDiagnostics(*ObservedSaves.Get())));
	UTrainingSaveGameSubsystem* Saves = ObservedSaves.Get();
	if (bBusy && (Saves->IsWaitingForIO() || Saves->IsIOComplete()) && !PreviewTicker.IsValid())
	{
		PreviewTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UTrainingSaveGamePanel::HandlePreviewElapsed), 1.0f);
	}
}

bool UTrainingSaveGamePanel::HandlePreviewElapsed(float DeltaSeconds)
{
	PreviewTicker.Reset();
	if (UTrainingSaveGameSubsystem* Saves = ObservedSaves.Get())
	{
		if (Saves->IsWaitingForIO()) { Saves->BeginPendingIO(); }
		else if (Saves->IsIOComplete()) { Saves->FinishOperation(); }
	}
	return false;
}

void UTrainingSaveGamePanel::HandleSaveClicked()
{
	ObservedState->SaveSession(false);
}

void UTrainingSaveGamePanel::HandleSaveAndQuitClicked()
{
	ObservedState->SaveSession(true);
}

bool UTrainingSaveGamePanel::HandleWindowCloseRequested()
{
	HandleSaveAndQuitClicked();
	return false;
}
