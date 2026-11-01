#include "TrainingSaveGameUI.h"
#include "TrainingSaveGameSubsystem.h"
#include "TrainingDebugHUD.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "RenderingThread.h"
#include "Misc/App.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

void UTrainingSaveGameUI::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	SaveSubsystem = Collection.InitializeDependency<UTrainingSaveGameSubsystem>();
	SaveSubsystem->OnStatusChanged.AddUObject(this, &UTrainingSaveGameUI::HandleStatusChanged);
	SaveSubsystem->OnBeforeIO.AddUObject(this, &UTrainingSaveGameUI::PaintPendingUI);
}

void UTrainingSaveGameUI::Deinitialize()
{
	SaveSubsystem->OnStatusChanged.RemoveAll(this);
	SaveSubsystem->OnBeforeIO.RemoveAll(this);
	CloseModal();
	Super::Deinitialize();
}

void UTrainingSaveGameUI::HandleStatusChanged(const FString& Status, bool bBusy)
{
	if (bBusy)
	{
		ShowModal();
	}
	else
	{
		CloseModal();
	}
}

void UTrainingSaveGameUI::ShowModal()
{
	if (!FSlateApplication::IsInitialized() || !FApp::CanEverRender())
	{
		return;
	}
	Modal = SNew(SWindow).Title(FText::FromString(TEXT("Save-game diagnostics")))
		.ClientSize(FVector2D(760.0f, 260.0f)).SupportsMaximize(false).SupportsMinimize(false).HasCloseButton(false)
		[
			SNew(SBorder).Padding(24.0f)
			[
				SNew(STextBlock).AutoWrapText(true)
				.Text(FText::FromString(UTrainingDebugHUD::FormatSaveDiagnostics(*SaveSubsystem)))
			]
		];
	UGameViewportClient* Viewport = GetGameInstance()->GetGameViewportClient();
	FSlateApplication& Slate = FSlateApplication::Get();
	Slate.AddModalWindow(Modal.ToSharedRef(), Viewport ? Viewport->GetWindow() : TSharedPtr<SWindow>(), true);
}

void UTrainingSaveGameUI::PaintPendingUI()
{
	if (!Modal.IsValid())
	{
		return;
	}
	FSlateApplication& Slate = FSlateApplication::Get();
	// Paint the diagnostic state before synchronous I/O blocks the game thread.
	Slate.Tick();
	Slate.GetRenderer()->Sync();
	FlushRenderingCommands();
}

void UTrainingSaveGameUI::CloseModal()
{
	if (Modal.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().RequestDestroyWindow(Modal.ToSharedRef());
	}
	Modal.Reset();
}
