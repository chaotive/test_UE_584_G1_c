#include "FeatureMainMenu.h"
#include "Features/GameWorkflow/FeatureGameFlow.h"
#include "Features_NonUE/FeatureSaveGameStorage.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GeneralProjectSettings.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"
#include "UObject/WeakObjectPtrTemplates.h"

#define LOCTEXT_NAMESPACE "FeatureMainMenu"

UFeatureMainMenu* UFeatureMainMenu::Present(UGameInstance& GameInstance, EFeatureGameFlow Flow)
{
	UFeatureMainMenu* Menu = CreateWidget<UFeatureMainMenu>(&GameInstance);
	Menu->BuildMenu(Flow);
	Menu->AddToViewport(20);
	if (APlayerController* Controller = GameInstance.GetFirstLocalPlayerController())
	{
		Controller->bShowMouseCursor = true;
		Controller->SetInputMode(FInputModeUIOnly().SetWidgetToFocus(Menu->TakeWidget()));
	}
	else
	{
		Menu->SetKeyboardFocus();
	}
	return Menu;
}

void UFeatureMainMenu::Dismiss()
{
	RemoveFromParent();
	if (APlayerController* Controller = GetGameInstance()->GetFirstLocalPlayerController())
	{
#if UE_BUILD_SHIPPING
		Controller->bShowMouseCursor = false;
		Controller->SetInputMode(FInputModeGameOnly());
#else
		Controller->SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));
#endif
	}
}

FReply UFeatureMainMenu::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& KeyEvent)
{
	if (bPauseMenu && KeyEvent.GetKey() == EKeys::Escape)
	{
		if (!KeyEvent.IsRepeat())
		{
			HandleResume();
		}
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(Geometry, KeyEvent);
}

void UFeatureMainMenu::BuildMenu(EFeatureGameFlow Flow)
{
	bPauseMenu = Flow == EFeatureGameFlow::PauseMenu;
	SetIsFocusable(true);
	UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
	WidgetTree->RootWidget = Background;
	Background->SetBrushColor(FLinearColor(0.012f, 0.022f, 0.035f));
	Background->SetHorizontalAlignment(HAlign_Center);
	Background->SetVerticalAlignment(VAlign_Center);

	UBorder* Card = WidgetTree->ConstructWidget<UBorder>();
	Card->SetBrushColor(FLinearColor(0.035f, 0.065f, 0.09f));
	Card->SetPadding(FMargin(52.0f, 40.0f));
	Background->SetContent(Card);

	UVerticalBox* Rows = WidgetTree->ConstructWidget<UVerticalBox>();
	Card->SetContent(Rows);

	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
	FText Heading = FText::FromString(GetDefault<UGeneralProjectSettings>()->ProjectName);
	switch (Flow)
	{
	case EFeatureGameFlow::PauseMenu:
		Heading = LOCTEXT("Paused", "Paused");
		break;
	case EFeatureGameFlow::Victory:
		Heading = LOCTEXT("Victory", "Victory");
		break;
	case EFeatureGameFlow::Lose:
		Heading = LOCTEXT("Defeat", "Defeat");
		break;
	default:
		break;
	}
	Title->SetText(Heading);
	Title->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 38));
	Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.38f, 0.90f, 1.0f)));
	Title->SetJustification(ETextJustify::Center);
	Rows->AddChildToVerticalBox(Title)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 24.0f));

	const bool bStartupMenu = Flow == EFeatureGameFlow::StartupMenu;
	const bool bMainMenu = bStartupMenu || bPauseMenu;
	const bool bHasSave = bMainMenu && FFeatureSaveGameStorage::HasSave();
	if (bStartupMenu)
	{
		AddAction(*Rows, LOCTEXT("NewGame", "New Game"))->OnClicked.AddDynamic(this, &UFeatureMainMenu::HandleNewGame);
		UButton* ContinueButton = AddAction(*Rows, LOCTEXT("Continue", "Continue"));
		ContinueButton->SetIsEnabled(bHasSave);
		ContinueButton->OnClicked.AddDynamic(this, &UFeatureMainMenu::HandleContinue);
	}
	else if (bPauseMenu)
	{
		AddAction(*Rows, LOCTEXT("Resume", "Resume Game"))->OnClicked.AddDynamic(this, &UFeatureMainMenu::HandleResume);
		AddAction(*Rows, LOCTEXT("ReturnToMenu", "Main Menu"))->OnClicked.AddDynamic(this, &UFeatureMainMenu::HandleReturnToMenu);
	}
	else
	{
		AddAction(*Rows, LOCTEXT("Continue", "Continue"))->OnClicked.AddDynamic(this, &UFeatureMainMenu::HandleReturnToMenu);
	}
	AddAction(*Rows, LOCTEXT("Exit", "Exit"))->OnClicked.AddDynamic(this, &UFeatureMainMenu::HandleExit);
	if (bMainMenu)
	{
		AddSaveSummary(*Rows, bHasSave);
	}
}

void UFeatureMainMenu::AddSaveSummary(UVerticalBox& Rows, bool bHasSave)
{
	UTextBlock* Summary = WidgetTree->ConstructWidget<UTextBlock>();
	Summary->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 14));
	Summary->SetJustification(ETextJustify::Center);
	Rows.AddChildToVerticalBox(Summary)->SetPadding(FMargin(0.0f, 16.0f, 0.0f, 0.0f));

	if (!bHasSave)
	{
		Summary->SetText(LOCTEXT("NoSavedGame", "No saved game."));
		return;
	}

	Summary->SetText(LOCTEXT("ReadingSavedGame", "Reading saved game..."));
	const TWeakObjectPtr<UTextBlock> WeakSummary(Summary);
	FFeatureSaveGameStorage::LoadAsync([WeakSummary](TOptional<FFeatureGameProgress> Progress, const FString&)
	{
		if (UTextBlock* Label = WeakSummary.Get())
		{
			Label->SetText(Progress.IsSet()
				? FText::Format(LOCTEXT("SavedTotalBounces", "Saved total bounces: {0}"), FText::AsNumber(Progress.GetValue().TotalBounces))
				: LOCTEXT("UnreadableSavedGame", "Saved game could not be read."));
		}
	});
}

UButton* UFeatureMainMenu::AddAction(UVerticalBox& Rows, const FText& Label)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>();
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>();
	Text->SetText(Label);
	Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 19));
	Text->SetJustification(ETextJustify::Center);
	Button->SetContent(Text);
	Rows.AddChildToVerticalBox(Button)->SetPadding(FMargin(0.0f, 5.0f));
	return Button;
}

void UFeatureMainMenu::HandleNewGame()
{
	OnAction.Broadcast(EFeatureMenuAction::NewGame);
}

void UFeatureMainMenu::HandleContinue()
{
	OnAction.Broadcast(EFeatureMenuAction::Continue);
}

void UFeatureMainMenu::HandleResume()
{
	OnAction.Broadcast(EFeatureMenuAction::Resume);
}

void UFeatureMainMenu::HandleReturnToMenu()
{
	OnAction.Broadcast(EFeatureMenuAction::ReturnToMenu);
}

void UFeatureMainMenu::HandleExit()
{
	OnAction.Broadcast(EFeatureMenuAction::Exit);
}

#undef LOCTEXT_NAMESPACE
