#include "FeatureMainMenu.h"
#include "Features/UI/Widgets/FeatureSavedProgressPanel.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GeneralProjectSettings.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"

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

void UFeatureMainMenu::NativeConstruct()
{
	Super::NativeConstruct();
	if (SavedProgressPanel)
	{
		SavedProgressPanel->OnPreviewChanged.AddUObject(this, &UFeatureMainMenu::UpdateSaveActions);
		UpdateSaveActions(SavedProgressPanel->GetSavedLevel(), SavedProgressPanel->HasSaveSlot());
	}
}

void UFeatureMainMenu::NativeDestruct()
{
	if (SavedProgressPanel)
	{
		SavedProgressPanel->OnPreviewChanged.RemoveAll(this);
	}
	Super::NativeDestruct();
}

FReply UFeatureMainMenu::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& KeyEvent)
{
	if ((bPauseMenu || bExitOnEscape) && KeyEvent.GetKey() == EKeys::Escape)
	{
		if (!KeyEvent.IsRepeat())
		{
			if (bPauseMenu)
			{
				HandleResume();
			}
			else
			{
				HandleExit();
			}
		}
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(Geometry, KeyEvent);
}

void UFeatureMainMenu::BuildMenu(EFeatureGameFlow Flow)
{
	const FFeatureMenuView View = FFeatureMenuViews::ForFlow(Flow);
	bPauseMenu = View.bPauseMenu;
	bExitOnEscape = View.bExitOnEscape;
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

	BuildHeader(*Rows, View);

	for (const FFeatureMenuItem& Item : View.Items)
	{
		AddViewAction(*Rows, Item);
	}
	if (View.bShowSaveSummary)
	{
		AddSaveSummary(*Rows, Flow == EFeatureGameFlow::StartupMenu);
	}
}

void UFeatureMainMenu::BuildHeader(UVerticalBox& Rows, const FFeatureMenuView& View)
{
	const UGeneralProjectSettings* Settings = GetDefault<UGeneralProjectSettings>();
	LogoTexture = FImageUtils::ImportFileAsTexture2D(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("FeatureBrand/FeatureCrazyBallsLogo.png")));
	UImage* Logo = WidgetTree->ConstructWidget<UImage>();
	Logo->SetBrushFromTexture(LogoTexture);
	USizeBox* LogoBox = WidgetTree->ConstructWidget<USizeBox>();
	LogoBox->SetWidthOverride(210.0f);
	LogoBox->SetHeightOverride(140.0f);
	LogoBox->SetContent(Logo);
	Rows.AddChildToVerticalBox(LogoBox)->SetHorizontalAlignment(HAlign_Center);

	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
	Title->SetText(FText::FromString(Settings->ProjectName));
	Title->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 32));
	Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.38f, 0.90f, 1.0f)));
	Title->SetJustification(ETextJustify::Center);
	Rows.AddChildToVerticalBox(Title);

	UTextBlock* Subtitle = WidgetTree->ConstructWidget<UTextBlock>();
	Subtitle->SetText(FText::FromString(Settings->Description));
	Subtitle->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 14));
	Subtitle->SetJustification(ETextJustify::Center);
	Rows.AddChildToVerticalBox(Subtitle)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));

	if (!View.Heading.IsEmpty())
	{
		UTextBlock* Heading = WidgetTree->ConstructWidget<UTextBlock>();
		Heading->SetText(View.Heading);
		Heading->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 26));
		Heading->SetColorAndOpacity(FSlateColor(FLinearColor(0.38f, 0.90f, 1.0f)));
		Heading->SetJustification(ETextJustify::Center);
		Rows.AddChildToVerticalBox(Heading)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	}
}

void UFeatureMainMenu::AddViewAction(UVerticalBox& Rows, const FFeatureMenuItem& Item)
{
	UButton* Button = AddAction(Rows, Item.Label);
	switch (Item.Action)
	{
	case EFeatureMenuAction::NewGame:
		Button->OnClicked.AddDynamic(this, &UFeatureMainMenu::HandleNewGame);
		break;
	case EFeatureMenuAction::Continue:
		ContinueButton = Button;
		DefaultContinueLabel = Item.Label;
		Button->SetIsEnabled(false);
		Button->OnClicked.AddDynamic(this, &UFeatureMainMenu::HandleContinue);
		break;
	case EFeatureMenuAction::Resume:
		Button->OnClicked.AddDynamic(this, &UFeatureMainMenu::HandleResume);
		break;
	case EFeatureMenuAction::ReturnToMenu:
		Button->OnClicked.AddDynamic(this, &UFeatureMainMenu::HandleReturnToMenu);
		break;
	case EFeatureMenuAction::Retry:
		Button->OnClicked.AddDynamic(this, &UFeatureMainMenu::HandleRetry);
		break;
	case EFeatureMenuAction::DeleteSave:
		DeleteSaveButton = Button;
		Button->SetIsEnabled(false);
		Button->OnClicked.AddDynamic(this, &UFeatureMainMenu::HandleDeleteSave);
		break;
	case EFeatureMenuAction::Exit:
		Button->OnClicked.AddDynamic(this, &UFeatureMainMenu::HandleExit);
		break;
	}
}

void UFeatureMainMenu::AddSaveSummary(UVerticalBox& Rows, bool bInitialMenu)
{
	SavedProgressPanel = CreateWidget<UFeatureSavedProgressPanel>(GetGameInstance());
	SavedProgressPanel->Configure(bInitialMenu);
	Rows.AddChildToVerticalBox(SavedProgressPanel)->SetPadding(FMargin(0.0f, 16.0f, 0.0f, 0.0f));
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

void UFeatureMainMenu::HandleRetry()
{
	OnAction.Broadcast(EFeatureMenuAction::Retry);
}

void UFeatureMainMenu::HandleDeleteSave()
{
	SavedProgressPanel->DeleteSave();
}

void UFeatureMainMenu::UpdateSaveActions(const TOptional<int32>& SavedLevel, bool bHasSave)
{
	if (ContinueButton)
	{
		ContinueButton->SetIsEnabled(SavedLevel.IsSet());
		UTextBlock* Label = Cast<UTextBlock>(ContinueButton->GetContent());
		Label->SetText(SavedLevel.IsSet()
			? FText::Format(LOCTEXT("ContinueLevel", "Continue on level {0}"), FText::AsNumber(SavedLevel.GetValue()))
			: DefaultContinueLabel);
	}
	if (DeleteSaveButton)
	{
		DeleteSaveButton->SetIsEnabled(bHasSave);
	}
}

void UFeatureMainMenu::HandleExit()
{
	OnAction.Broadcast(EFeatureMenuAction::Exit);
}

#undef LOCTEXT_NAMESPACE
