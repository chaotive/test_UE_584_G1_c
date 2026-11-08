#pragma once

#include "CoreMinimal.h"
#include "Features/GameWorkflow/FeatureGameFlow.h"
#include "FeatureMenuActions.h"
#include "Subsystems/WorldSubsystem.h"
#include "FeatureMenuController.generated.h"

class UFeatureMainMenu;

/** Presents flow menus and executes their actions without knowing the project GameMode. */
UCLASS()
class TESTFEATURES_API UFeatureMenuController : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	DECLARE_EVENT_OneParam(UFeatureMenuController, FStartRequestedEvent, bool);

	// Requests a fresh or continued session from the configured gameplay composition root.
	// The native event copies the continuation bool and stores weak UObject bindings.
	FStartRequestedEvent OnStartRequested;

	// Replaces the displayed menu to match the authoritative game flow.
	// Flow is copied by value; this subsystem retains the created widget through UPROPERTY.
	void PresentFlow(EFeatureGameFlow Flow);

	// Executes a menu intent through session, persistence, or engine navigation APIs.
	// Action is copied by value without pointer or reference ownership.
	void Execute(EFeatureMenuAction Action);

	// Reads the flow currently presented by this UI controller.
	// Returns the enum by value without pointer or reference ownership.
	EFeatureGameFlow GetFlowState() const;

	// Removes menu subscriptions and its widget when the world shuts down.
	// No pointer or reference parameters or return value; releases the retained widget.
	virtual void Deinitialize() override;

private:
	// Applies the New Game or Continue save policy before requesting session startup.
	// bContinue is copied by value; no pointer or reference ownership is transferred.
	void StartSession(bool bContinue);

	// Exits directly from startup or saves the active progress before quitting.
	// No pointer or reference parameters or return value; native systems perform the operation.
	void ExitGame();

	// Retains the active menu and owns its action subscription.
	// UPROPERTY tracks the strong widget reference for Unreal garbage collection.
	UPROPERTY()
	TObjectPtr<UFeatureMainMenu> FlowMenu;

	// Retains the displayed flow to select startup-specific exit behavior.
	// The enum is a UI snapshot passed by value from the authoritative GameMode.
	EFeatureGameFlow PresentedFlow = EFeatureGameFlow::StartupMenu;
};
