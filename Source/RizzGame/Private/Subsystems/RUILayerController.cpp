// RUILayerController.cpp
#include "Subsystems/RUILayerController.h"

#include "Subsystems/REventBusSubsystem.h"
#include "Events/CombatEvents.h"

void URUILayerController::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (auto* Bus = GetGameInstance()->GetSubsystem<UREventBusSubsystem>())
	{
		Bus->Subscribe<FCombatStarted>(this, &URUILayerController::OnCombatStarted);
		Bus->Subscribe<FCombatEnded>(this, &URUILayerController::OnCombatEnded);
	}
}

void URUILayerController::Deinitialize()
{
	if (auto* Bus = GetGameInstance()->GetSubsystem<UREventBusSubsystem>())
		Bus->UnsubscribeAll(this);
	Super::Deinitialize();
}

void URUILayerController::RegisterGameHUD(URGameHUD* InHUD, APlayerController* OwningPC)
{
	GameHUD = InHUD;
	CachedPC = OwningPC;
	bInCombat = false;
	ApplyDefaultLayerState();
}

URGameHUD* URUILayerController::GetGameHUD() const
{
	return GameHUD.Get();
}

void URUILayerController::ApplyDefaultLayerState()
{
	if (auto* HUD = GameHUD.Get())
	{
		HUD->SetLayerVisible(ERUILayer::Exploration, true);
		HUD->SetLayerVisible(ERUILayer::Combat, false);
		HUD->SetLayerVisible(ERUILayer::Overlay, true);
		HUD->SetLayerVisible(ERUILayer::Popup, false);
	}
	UpdateInputMode();
}

void URUILayerController::OnCombatStarted(const FCombatStarted& Event)
{
	bInCombat = true;
	if (auto* HUD = GameHUD.Get())
		HUD->SetLayerVisible(ERUILayer::Combat, true);
	UpdateInputMode();
}

void URUILayerController::OnCombatEnded(const FCombatEnded& Event)
{
	bInCombat = false;
	if (auto* HUD = GameHUD.Get())
		HUD->SetLayerVisible(ERUILayer::Combat, false);
	UpdateInputMode();
}

bool URUILayerController::HasActivePopup() const
{
	auto* HUD = GameHUD.Get();
	if (!HUD) return false;
	auto* Layer = HUD->GetLayer(ERUILayer::Popup);
	return Layer && Layer->GetChildrenCount() > 0;
}

void URUILayerController::ShowPopup(UUserWidget* PopupWidget)
{
	auto* HUD = GameHUD.Get();
	if (!HUD || !PopupWidget) return;

	const bool bWasEmpty = !HasActivePopup();

	if (auto* PopupLayer = HUD->GetLayer(ERUILayer::Popup))
		PopupLayer->AddChild(PopupWidget);

	if (bWasEmpty)
	{
		HUD->SetLayerVisible(ERUILayer::Popup, true, ESlateVisibility::Visible);
		UpdateInputMode();
	}
}

void URUILayerController::ClosePopup(UUserWidget* PopupWidget)
{
	auto* HUD = GameHUD.Get();
	if (!PopupWidget) return;

	PopupWidget->RemoveFromParent();

	if (!HasActivePopup() && HUD)
	{
		HUD->SetLayerVisible(ERUILayer::Popup, false);
		UpdateInputMode();
	}
}

void URUILayerController::UpdateInputMode()
{
	auto* PC = CachedPC.Get();
	if (!PC) return;

	if (HasActivePopup())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
	else if (bInCombat)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
	else
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
}
