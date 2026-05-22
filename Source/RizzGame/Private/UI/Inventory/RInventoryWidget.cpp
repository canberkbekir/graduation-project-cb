// RInventoryWidget.cpp

#include "UI/Inventory/RInventoryWidget.h"
#include "UI/Inventory/RInventoryItemWidget.h"
#include "UI/Inventory/RInventoryGridWidget.h"
#include "UI/Inventory/RQuickWeaponBarWidget.h"
#include "Components/REquipmentComponent.h"
#include "Components/PanelWidget.h"
#include "Components/ComboBoxString.h"

void URInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetupSortDropdown();
}

void URInventoryWidget::NativeDestruct()
{
    // Unbind from inventory
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveDynamic(this, &URInventoryWidget::HandleInventoryChanged);
    }

    // Unbind sort dropdown
    if (CB_SortDropdown)
    {
        CB_SortDropdown->OnSelectionChanged.RemoveDynamic(this, &URInventoryWidget::HandleSortDropdownChanged);
    }

    Super::NativeDestruct();
}

void URInventoryWidget::Init(URInventoryComponent* InInventory, UREquipmentComponent* InEquipment)
{
    if (Inventory == InInventory && Equipment == InEquipment)
    {
        return;
    }

    // Unbind previous inventory
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveDynamic(this, &URInventoryWidget::HandleInventoryChanged);
    }

    Inventory = InInventory;
    Equipment = InEquipment;

    if (Inventory)
    {
        Inventory->OnInventoryChanged.AddDynamic(this, &URInventoryWidget::HandleInventoryChanged);

        // Apply current sort settings
        Inventory->SortInventory(SortKey, SortDirection);
    }

    // Initialize grid widget if available
    if (InventoryGrid)
    {
        InventoryGrid->Init(Inventory);
    }
    else
    {
        // Fall back to legacy mode
        RebuildItems();
    }

    // Initialize quick weapon bar if available
    if (QuickWeaponBar && Equipment)
    {
        QuickWeaponBar->Init(Equipment);
    }
}

void URInventoryWidget::InitLegacy(URInventoryComponent* InInventory)
{
    Init(InInventory, nullptr);
}

void URInventoryWidget::HandleInventoryChanged(URInventoryComponent* ChangedInventory)
{
    if (ChangedInventory != Inventory)
    {
        return;
    }

    // Grid widget handles its own updates
    // Only rebuild for legacy mode
    if (!InventoryGrid)
    {
        RebuildItems();
    }
}

void URInventoryWidget::RebuildItems()
{
    if (!ItemsRoot || !ItemWidgetClass || !Inventory)
    {
        return;
    }

    ItemsRoot->ClearChildren();

    const TArray<FRInventoryEntry>& EntriesRef = Inventory->GetItems();

    for (int32 Index = 0; Index < EntriesRef.Num(); ++Index)
    {
        const FRInventoryEntry& Entry = EntriesRef[Index];
        if (!Entry.IsValidEntry())
        {
            continue;
        }

        URInventoryItemWidget* Row = CreateWidget<URInventoryItemWidget>(this, ItemWidgetClass);
        if (!Row)
        {
            continue;
        }

        Row->SetupFromEntry(Entry);
        Row->SetInventoryIndex(Index);

        ItemsRoot->AddChild(Row);
    }
}

void URInventoryWidget::SetSort(ERInventorySortKey InKey, ERInventorySortDirection InDirection)
{
    SortKey = InKey;
    SortDirection = InDirection;

    if (Inventory)
    {
        Inventory->SortInventory(SortKey, SortDirection);
    }

    if (InventoryGrid)
    {
        InventoryGrid->SetSort(SortKey, SortDirection);
    }
}

void URInventoryWidget::ToggleSortDirection()
{
    SortDirection = (SortDirection == ERInventorySortDirection::Ascending)
                        ? ERInventorySortDirection::Descending
                        : ERInventorySortDirection::Ascending;

    if (Inventory)
    {
        Inventory->SortInventory(SortKey, SortDirection);
    }

    if (InventoryGrid)
    {
        InventoryGrid->ToggleSortDirection();
    }
}

void URInventoryWidget::MoveItem(int32 FromIndex, int32 ToIndex)
{
    if (Inventory)
    {
        Inventory->MoveEntry(FromIndex, ToIndex);
    }
}

void URInventoryWidget::SwapItems(int32 IndexA, int32 IndexB)
{
    if (Inventory)
    {
        Inventory->SwapEntries(IndexA, IndexB);
    }
}

void URInventoryWidget::PlaySlideInAnimation()
{
    BP_PlaySlideInAnimation();
}

void URInventoryWidget::PlaySlideOutAnimation()
{
    BP_PlaySlideOutAnimation();
}

void URInventoryWidget::SetupSortDropdown()
{
    if (!CB_SortDropdown)
    {
        return;
    }

    // Add sort options
    CB_SortDropdown->ClearOptions();
    CB_SortDropdown->AddOption(TEXT("Name"));
    CB_SortDropdown->AddOption(TEXT("Type"));
    CB_SortDropdown->AddOption(TEXT("Weight"));
    CB_SortDropdown->AddOption(TEXT("Latest"));

    // Set default selection
    CB_SortDropdown->SetSelectedOption(TEXT("Name"));

    // Bind selection change
    CB_SortDropdown->OnSelectionChanged.AddDynamic(this, &URInventoryWidget::HandleSortDropdownChanged);
}

void URInventoryWidget::HandleSortDropdownChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (SelectionType == ESelectInfo::Direct)
    {
        return; // Ignore programmatic changes
    }

    ERInventorySortKey NewSortKey = ERInventorySortKey::Name;

    if (SelectedItem == TEXT("Name"))
    {
        NewSortKey = ERInventorySortKey::Name;
    }
    else if (SelectedItem == TEXT("Type"))
    {
        NewSortKey = ERInventorySortKey::Type;
    }
    else if (SelectedItem == TEXT("Weight"))
    {
        NewSortKey = ERInventorySortKey::Weight;
    }
    else if (SelectedItem == TEXT("Latest"))
    {
        NewSortKey = ERInventorySortKey::Latest;
    }

    SetSort(NewSortKey, SortDirection);
}
