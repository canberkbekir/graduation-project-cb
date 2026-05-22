// RQuickWeaponBarWidget.cpp

#include "UI/Inventory/RQuickWeaponBarWidget.h"
#include "UI/CharacterSheet/REquipmentSlotWidget.h"
#include "Components/REquipmentComponent.h"
#include "Items/RItemDefinition.h"
#include "Components/PanelWidget.h"

void URQuickWeaponBarWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    // Discover slots placed in Blueprint
    DiscoverSlots();

    // Show editor preview when designing
    if (IsDesignTime() && bShowEditorPreview)
    {
        CreateEditorPreview();
    }
}

void URQuickWeaponBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Discover slots at runtime if not already done
    if (SlotWidgets.Num() == 0)
    {
        DiscoverSlots();
    }

    BindSlotEvents();
}

void URQuickWeaponBarWidget::NativeDestruct()
{
    UnbindSlotEvents();
    Super::NativeDestruct();
}

void URQuickWeaponBarWidget::DiscoverSlots()
{
    SlotWidgets.Empty();

    if (!SlotsContainer)
    {
        return;
    }

    // Find all UREquipmentSlotWidget children in the container
    for (int32 i = 0; i < SlotsContainer->GetChildrenCount(); ++i)
    {
        UWidget* Child = SlotsContainer->GetChildAt(i);
        if (UREquipmentSlotWidget* SlotWidget = Cast<UREquipmentSlotWidget>(Child))
        {
            SlotWidgets.Add(SlotWidget);
        }
    }
}

void URQuickWeaponBarWidget::CreateEditorPreview()
{
    for (int32 i = 0; i < SlotWidgets.Num(); ++i)
    {
        if (!SlotWidgets[i])
        {
            continue;
        }

        // Show as filled preview for first N slots
        if (i < EditorPreviewFilledSlots)
        {
            SlotWidgets[i]->SetupAsEditorPreview();
        }
        else
        {
            SlotWidgets[i]->ClearItem();
        }

        // Show selected state for preview selected index
        SlotWidgets[i]->SetToggled(i == EditorPreviewSelectedIndex, false);
    }
}

void URQuickWeaponBarWidget::BindSlotEvents()
{
    for (UREquipmentSlotWidget* SlotWidget : SlotWidgets)
    {
        if (SlotWidget)
        {
            SlotWidget->OnClicked.AddDynamic(this, &URQuickWeaponBarWidget::HandleSlotClicked);
        }
    }
}

void URQuickWeaponBarWidget::UnbindSlotEvents()
{
    for (UREquipmentSlotWidget* SlotWidget : SlotWidgets)
    {
        if (SlotWidget)
        {
            SlotWidget->OnClicked.RemoveDynamic(this, &URQuickWeaponBarWidget::HandleSlotClicked);
        }
    }
}

void URQuickWeaponBarWidget::Init(UREquipmentComponent* InEquipmentComponent)
{
    EquipmentComponent = InEquipmentComponent;

    // Re-discover slots in case they changed
    if (SlotWidgets.Num() == 0)
    {
        DiscoverSlots();
        BindSlotEvents();
    }

    Refresh();
}

void URQuickWeaponBarWidget::Refresh()
{
    if (!EquipmentComponent)
    {
        return;
    }

    for (int32 i = 0; i < SlotWidgets.Num(); ++i)
    {
        if (!SlotWidgets[i])
        {
            continue;
        }

        URItemDefinition* WeaponDef = EquipmentComponent->GetQuickWeaponSlot(i);
        SlotWidgets[i]->SetItem(WeaponDef);

        // Update toggle state for selected slot
        SlotWidgets[i]->SetToggled(i == SelectedSlotIndex, false);
    }
}

UREquipmentSlotWidget* URQuickWeaponBarWidget::GetSlotAt(int32 Index) const
{
    if (Index >= 0 && Index < SlotWidgets.Num())
    {
        return SlotWidgets[Index];
    }
    return nullptr;
}

void URQuickWeaponBarWidget::SetQuickSlot(int32 SlotIndex, URItemDefinition* ItemDef)
{
    if (EquipmentComponent && SlotIndex >= 0 && SlotIndex < SlotWidgets.Num())
    {
        EquipmentComponent->SetQuickWeaponSlot(SlotIndex, ItemDef);

        if (SlotWidgets[SlotIndex])
        {
            SlotWidgets[SlotIndex]->SetItem(ItemDef);
        }
    }
}

void URQuickWeaponBarWidget::EquipFromSlot(int32 SlotIndex)
{
    if (EquipmentComponent && SlotIndex >= 0 && SlotIndex < SlotWidgets.Num())
    {
        SelectSlot(SlotIndex);
        EquipmentComponent->EquipFromQuickSlot(SlotIndex);
    }
}

void URQuickWeaponBarWidget::SelectSlot(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= SlotWidgets.Num())
    {
        return;
    }

    // Deselect previous
    if (SelectedSlotIndex >= 0 && SelectedSlotIndex < SlotWidgets.Num() && SlotWidgets[SelectedSlotIndex])
    {
        SlotWidgets[SelectedSlotIndex]->SetToggled(false, false);
    }

    SelectedSlotIndex = SlotIndex;

    // Select new
    if (SlotWidgets[SelectedSlotIndex])
    {
        SlotWidgets[SelectedSlotIndex]->SetToggled(true, false);

        URItemDefinition* ItemDef = EquipmentComponent ? EquipmentComponent->GetQuickWeaponSlot(SelectedSlotIndex) : nullptr;
        OnQuickSlotSelected.Broadcast(SelectedSlotIndex, ItemDef);
    }
}

void URQuickWeaponBarWidget::HandleSlotClicked(const FPointerEvent& PointerEvent)
{
    // Find which slot was clicked
    for (int32 i = 0; i < SlotWidgets.Num(); ++i)
    {
        if (SlotWidgets[i] && SlotWidgets[i]->IsHovered())
        {
            EquipFromSlot(i);
            break;
        }
    }
}
