#include "FeatureSavedProgressPanel.h"
#include "Features/Gameplay/FeatureGameplaySessionResult.h"
#include "Features/GameWorkflow/FeatureSaveGame.h"
#include "Features/GameWorkflow/FeatureSaveGameSubsystem.h"
#include "Features_NonUE/FeatureSaveGameStorage.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "FeatureMainMenu"

void UFeatureSavedProgressPanel::Configure(bool bDetailed)
{
	bDetailedSummary = bDetailed;
}

void UFeatureSavedProgressPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SummaryText = WidgetTree->ConstructWidget<UTextBlock>();
	SummaryText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 14));
	SummaryText->SetJustification(ETextJustify::Center);
	WidgetTree->RootWidget = SummaryText;
}

void UFeatureSavedProgressPanel::NativeConstruct()
{
	Super::NativeConstruct();
	ObservedSaves = GetGameInstance()->GetSubsystem<UFeatureSaveGameSubsystem>();
	ObservedSaves->OnSaveDeleted.AddUObject(this, &UFeatureSavedProgressPanel::HandleSaveDeleted);
	RefreshPreview();
}

void UFeatureSavedProgressPanel::NativeDestruct()
{
	++PreviewGeneration;
	if (UFeatureSaveGameSubsystem* Saves = ObservedSaves.Get())
	{
		Saves->OnSaveDeleted.RemoveAll(this);
	}
	ObservedSaves.Reset();
	Super::NativeDestruct();
}

void UFeatureSavedProgressPanel::RefreshPreview()
{
	const uint32 RequestGeneration = ++PreviewGeneration;
	SavedLevel.Reset();
	bHasSaveSlot = FFeatureSaveGameStorage::HasSave();
	PublishPreview(bHasSaveSlot ? LOCTEXT("ReadingSavedGame", "Reading saved game...") : LOCTEXT("NoSavedGame", "No saved game."));
	if (!bHasSaveSlot)
	{
		return;
	}

	const TWeakObjectPtr<UFeatureSavedProgressPanel> WeakPanel(this);
	FFeatureSaveGameStorage::LoadAsync([WeakPanel, RequestGeneration](TOptional<FFeatureGameProgress> Progress, const FString&)
	{
		UFeatureSavedProgressPanel* Panel = WeakPanel.Get();
		if (!Panel || Panel->PreviewGeneration != RequestGeneration)
		{
			return;
		}
		if (Progress.IsSet())
		{
			Panel->SavedLevel = Progress->CurrentLevel;
			Panel->PublishPreview(Panel->FormatProgress(Progress.GetValue()));
		}
		else
		{
			Panel->PublishPreview(LOCTEXT("UnreadableSavedGame", "Saved game could not be read."));
		}
	});
}

FText UFeatureSavedProgressPanel::FormatProgress(const FFeatureGameProgress& Progress) const
{
	if (!bDetailedSummary)
	{
		return FText::Format(LOCTEXT("SavedTotalBounces", "Saved total bounces: {0}"), FText::AsNumber(Progress.TotalBounces));
	}

	FText Summary = FText::Format(LOCTEXT("SavedProgress", "Saved level: {0}\nTotal bounces: {1} | Sessions: {2}"),
		FText::AsNumber(Progress.CurrentLevel), FText::AsNumber(Progress.TotalBounces), FText::AsNumber(Progress.Sessions.Num()));
	if (!Progress.Sessions.IsEmpty())
	{
		const FFeatureGameplaySessionRecord& LastSession = Progress.Sessions.Last();
		FText Result = LOCTEXT("SessionInProgress", "In progress");
		if (LastSession.Result == EFeatureGameplaySessionResult::Victory)
		{
			Result = LOCTEXT("SessionVictory", "Victory");
		}
		else if (LastSession.Result == EFeatureGameplaySessionResult::Defeat)
		{
			Result = LOCTEXT("SessionDefeat", "Defeat");
		}
		Summary = FText::Format(LOCTEXT("SavedProgressWithSession", "{0}\nLast session: {1}\nPlayer contacts: {2} | Bounces: {3}"),
			Summary, Result, FText::AsNumber(LastSession.PlayerBallCollisions), FText::AsNumber(LastSession.TotalBounces));
	}
	return Summary;
}

void UFeatureSavedProgressPanel::PublishPreview(const FText& Summary)
{
	SummaryText->SetText(Summary);
	OnPreviewChanged.Broadcast(SavedLevel, bHasSaveSlot);
}

void UFeatureSavedProgressPanel::HandleSaveDeleted(bool bSucceeded)
{
	if (!bSucceeded)
	{
		PublishPreview(LOCTEXT("DeleteSaveFailed", "Could not delete saved game."));
		return;
	}
	++PreviewGeneration;
	SavedLevel.Reset();
	bHasSaveSlot = false;
	PublishPreview(LOCTEXT("NoSavedGame", "No saved game."));
}

const TOptional<int32>& UFeatureSavedProgressPanel::GetSavedLevel() const
{
	return SavedLevel;
}

bool UFeatureSavedProgressPanel::HasSaveSlot() const
{
	return bHasSaveSlot;
}

#undef LOCTEXT_NAMESPACE
