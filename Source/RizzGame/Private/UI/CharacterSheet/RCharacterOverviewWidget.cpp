// RCharacterOverviewWidget.cpp

#include "UI/CharacterSheet/RCharacterOverviewWidget.h"
#include "UI/CharacterSheet/RCharacterSheetWidget.h"
#include "UI/CharacterSheet/REquipmentPaperDollWidget.h"
#include "UI/CharacterSheet/RViewNavigationWidget.h"
#include "UI/CharacterSheet/RCharacterSheetActor.h"
#include "UI/Inventory/RInventoryWidget.h"
#include "Core/RCharacterBase.h"
#include "Components/RInventoryComponent.h"
#include "Components/REquipmentComponent.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

void URCharacterOverviewWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
}

void URCharacterOverviewWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind view navigation events
    if (ViewNavigation)
    {
        ViewNavigation->OnViewChanged.AddDynamic(this, &URCharacterOverviewWidget::HandleViewChanged);
    }
}

void URCharacterOverviewWidget::NativeDestruct()
{
    // Unbind view navigation events
    if (ViewNavigation)
    {
        ViewNavigation->OnViewChanged.RemoveDynamic(this, &URCharacterOverviewWidget::HandleViewChanged);
    }

    Super::NativeDestruct();
}

void URCharacterOverviewWidget::InitCharacterOverviewWidget(ARCharacterBase* InCharacter, bool bSetInputMode)
{
    CurrentCharacter = InCharacter;

    // Update character name
    UpdateCharacterName();

    // Initialize child panels
    InitializePanels();

    SetIsEnabled(true);
    SetVisibility(ESlateVisibility::Visible);

    // Play open animation
    PlayPanelOpenAnimation();

    BP_OnOpened(CurrentCharacter);

    if (bSetInputMode)
    {
        if (APlayerController* PC = GetOwningPlayer())
        {
            PC->bShowMouseCursor = true;
            PC->bEnableClickEvents = true;
            PC->bEnableMouseOverEvents = true;

            FInputModeUIOnly Mode;
            Mode.SetWidgetToFocus(TakeWidget());
            Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(Mode);
        }
    }

    SetKeyboardFocus();
}

void URCharacterOverviewWidget::CloseCharacterOverviewWidget()
{
    // Play close animation
    PlayPanelCloseAnimation();

    BP_OnClosed();

    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->bShowMouseCursor = false;
        FInputModeGameOnly Mode;
        PC->SetInputMode(Mode);
    }

    SetIsEnabled(false);
    SetVisibility(ESlateVisibility::Collapsed);
    CurrentCharacter = nullptr;
}

void URCharacterOverviewWidget::SetPreviewActor(ARCharacterSheetActor* InPreviewActor)
{
    PreviewActor = InPreviewActor;

    // Update center panel if already initialized
    if (CenterPanel && CurrentCharacter && PreviewActor)
    {
        CenterPanel->Init(CurrentCharacter, PreviewActor);
    }
}

void URCharacterOverviewWidget::InitializePanels()
{
    if (!CurrentCharacter)
    {
        return;
    }

    // Initialize character sheet panel
    if (CharSheetPanel)
    {
        CharSheetPanel->InitFromCharacter(CurrentCharacter);
    }

    // Initialize center panel (paper doll)
    if (CenterPanel && PreviewActor)
    {
        CenterPanel->Init(CurrentCharacter, PreviewActor);
    }

    // Initialize inventory panel
    if (InventoryPanel)
    {
        URInventoryComponent* Inventory = CurrentCharacter->GetInventoryComponent();
        UREquipmentComponent* Equipment = CurrentCharacter->GetEquipmentComponent();
        InventoryPanel->Init(Inventory, Equipment);
    }
}

void URCharacterOverviewWidget::UpdateCharacterName()
{
    if (Txt_CharacterName && CurrentCharacter)
    {
        Txt_CharacterName->SetText(CurrentCharacter->GetDisplayName());
    }
}

void URCharacterOverviewWidget::SwitchToView(FName ViewID)
{
    if (ViewNavigation)
    {
        ViewNavigation->NavigateToView(ViewID);
    }
}

FName URCharacterOverviewWidget::GetCurrentViewID() const
{
    if (ViewNavigation)
    {
        return ViewNavigation->GetCurrentViewID();
    }
    return NAME_None;
}

void URCharacterOverviewWidget::HandleViewChanged(FName NewViewID, const FText& DisplayName, int32 ViewIndex)
{
    // Update widget switcher if available
    if (ViewSwitcher)
    {
        ViewSwitcher->SetActiveWidgetIndex(ViewIndex);
    }

    BP_OnViewChanged(NewViewID, DisplayName);
}

void URCharacterOverviewWidget::PlayPanelOpenAnimation()
{
    // Trigger slide-in animations on child panels
    if (CharSheetPanel)
    {
        CharSheetPanel->PlaySlideInAnimation();
    }

    if (InventoryPanel)
    {
        InventoryPanel->PlaySlideInAnimation();
    }
}

void URCharacterOverviewWidget::PlayPanelCloseAnimation()
{
    // Trigger slide-out animations on child panels
    if (CharSheetPanel)
    {
        CharSheetPanel->PlaySlideOutAnimation();
    }

    if (InventoryPanel)
    {
        InventoryPanel->PlaySlideOutAnimation();
    }
}

FReply URCharacterOverviewWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    // Handle left/right arrow keys for view navigation
    if (ViewNavigation)
    {
        if (InKeyEvent.GetKey() == EKeys::Left || InKeyEvent.GetKey() == EKeys::A)
        {
            ViewNavigation->NavigateLeft();
            return FReply::Handled();
        }
        else if (InKeyEvent.GetKey() == EKeys::Right || InKeyEvent.GetKey() == EKeys::D)
        {
            ViewNavigation->NavigateRight();
            return FReply::Handled();
        }
    }

    // Handle escape to close
    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        CloseCharacterOverviewWidget();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
