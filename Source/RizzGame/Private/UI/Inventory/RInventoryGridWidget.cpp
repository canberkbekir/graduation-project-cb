// RInventoryGridWidget.cpp

#include "UI/Inventory/RInventoryGridWidget.h"
#include "UI/Inventory/RInventorySlotWidget.h"
#include "Items/RItemDefinition.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void URInventoryGridWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    // Show editor preview when designing
    if (IsDesignTime() && bShowEditorPreview)
    {
        CalculatedColumns = CalculateColumnsForWidth(EditorPreviewWidth);
        CreateEditorPreview();
    }
}

void URInventoryGridWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Clear editor preview and rebuild for runtime
    if (!IsDesignTime())
    {
        RecalculateColumns();
        RebuildGrid();
    }
}

void URInventoryGridWidget::NativeDestruct()
{
    // Unbind from inventory
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveDynamic(this, &URInventoryGridWidget::HandleInventoryChanged);
    }

    Super::NativeDestruct();
}

void URInventoryGridWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Check if width changed and recalculate if needed
    float CurrentWidth = MyGeometry.GetLocalSize().X;
    if (!FMath::IsNearlyEqual(CurrentWidth, LastKnownWidth, 1.0f) && CurrentWidth > 0)
    {
        LastKnownWidth = CurrentWidth;
        int32 NewColumns = CalculateColumnsForWidth(CurrentWidth);
        if (NewColumns != CalculatedColumns)
        {
            CalculatedColumns = NewColumns;
            RebuildGrid();
        }
    }
}

void URInventoryGridWidget::Init(URInventoryComponent* InInventory)
{
    // Unbind from previous inventory
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveDynamic(this, &URInventoryGridWidget::HandleInventoryChanged);
    }

    Inventory = InInventory;

    if (Inventory)
    {
        // Bind to inventory changes
        Inventory->OnInventoryChanged.AddDynamic(this, &URInventoryGridWidget::HandleInventoryChanged);

        // Apply current sort settings
        Inventory->SortInventory(SortKey, SortDirection);
    }

    // Build the grid
    RecalculateColumns();
    RebuildGrid();
}

void URInventoryGridWidget::RecalculateColumns()
{
    float Width = GetAvailableWidth();
    if (Width > 0)
    {
        CalculatedColumns = CalculateColumnsForWidth(Width);
        LastKnownWidth = Width;
    }
}

float URInventoryGridWidget::GetAvailableWidth() const
{
    if (IsDesignTime())
    {
        return EditorPreviewWidth;
    }

    // Try to get actual geometry
    if (GridPanel)
    {
        FGeometry Geometry = GridPanel->GetCachedGeometry();
        float Width = Geometry.GetLocalSize().X;
        if (Width > 0)
        {
            return Width;
        }
    }

    // Fallback: try this widget's geometry
    FGeometry Geometry = GetCachedGeometry();
    float Width = Geometry.GetLocalSize().X;
    if (Width > 0)
    {
        return Width;
    }

    // Default fallback
    return EditorPreviewWidth;
}

int32 URInventoryGridWidget::CalculateColumnsForWidth(float AvailableWidth) const
{
    if (SlotSize <= 0)
    {
        return 1;
    }

    // Each slot takes SlotSize + SlotSpacing total width (including padding on all sides)
    // So calculate directly: AvailableWidth / (SlotSize + SlotSpacing)
    float TotalSlotWidth = SlotSize + SlotSpacing;
    int32 Columns = FMath::FloorToInt(AvailableWidth / TotalSlotWidth);

    return FMath::Max(1, Columns);
}

void URInventoryGridWidget::RebuildGrid()
{
    if (!GridPanel || !SlotWidgetClass)
    {
        return;
    }

    // Configure grid panel spacing
    GridPanel->SetSlotPadding(FMargin(SlotSpacing * 0.5f));
    GridPanel->SetMinDesiredSlotWidth(SlotSize);
    GridPanel->SetMinDesiredSlotHeight(SlotSize);

    int32 RequiredSlots = CalculateRequiredSlots();
    EnsureSlotCount(RequiredSlots);
    UpdateSlotPositions();
    UpdateSlots();
}

void URInventoryGridWidget::RefreshSlots()
{
    int32 RequiredSlots = CalculateRequiredSlots();

    // Only recreate if slot count changed
    if (RequiredSlots != SlotWidgets.Num())
    {
        EnsureSlotCount(RequiredSlots);
        UpdateSlotPositions();
    }

    UpdateSlots();
}

int32 URInventoryGridWidget::CalculateRequiredSlots() const
{
    if (CalculatedColumns <= 0)
    {
        return MinVisibleRows > 0 ? MinVisibleRows : 1;
    }

    int32 ItemCount = GetItemCount();

    // Calculate rows needed for items
    int32 RowsNeeded = (ItemCount + CalculatedColumns - 1) / CalculatedColumns; // Ceiling division

    // Apply minimum visible rows
    RowsNeeded = FMath::Max(RowsNeeded, MinVisibleRows);

    // Ensure at least 1 row
    RowsNeeded = FMath::Max(RowsNeeded, 1);

    return RowsNeeded * CalculatedColumns;
}

void URInventoryGridWidget::EnsureSlotCount(int32 RequiredSlots)
{
    if (!GridPanel || !SlotWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("URInventoryGridWidget::EnsureSlotCount - Missing GridPanel or SlotWidgetClass"));
        return;
    }

    // Remove excess slots (need to remove the SizeBox wrapper, not just the slot)
    while (SlotWidgets.Num() > RequiredSlots)
    {
        URInventorySlotWidget* SlotToRemove = SlotWidgets.Pop();
        if (SlotToRemove)
        {
            SlotToRemove->OnSlotClicked.RemoveDynamic(this, &URInventoryGridWidget::HandleSlotClicked);

            // Remove the SizeBox wrapper (parent of the slot)
            USizeBox* SizeBoxWrapper = Cast<USizeBox>(SlotToRemove->GetParent());
            if (SizeBoxWrapper)
            {
                SizeBoxWrapper->RemoveFromParent();
            }
            else
            {
                SlotToRemove->RemoveFromParent();
            }
        }
    }

    // Add new slots if needed
    while (SlotWidgets.Num() < RequiredSlots)
    {
        URInventorySlotWidget* SlotWidget = CreateWidget<URInventorySlotWidget>(this, SlotWidgetClass);
        if (!SlotWidget)
        {
            continue;
        }

        // Create SizeBox wrapper to enforce square dimensions
        USizeBox* SizeBoxWrapper = NewObject<USizeBox>(this);
        SizeBoxWrapper->SetWidthOverride(SlotSize);
        SizeBoxWrapper->SetHeightOverride(SlotSize);

        // Add slot to SizeBox
        SizeBoxWrapper->AddChild(SlotWidget);

        // Add SizeBox to grid (position will be set in UpdateSlotPositions)
        GridPanel->AddChildToUniformGrid(SizeBoxWrapper, 0, 0);

        // Bind click event
        SlotWidget->OnSlotClicked.AddDynamic(this, &URInventoryGridWidget::HandleSlotClicked);

        // Store reference
        SlotWidgets.Add(SlotWidget);
    }
}

void URInventoryGridWidget::UpdateSlotPositions()
{
    if (CalculatedColumns <= 0)
    {
        return;
    }

    for (int32 i = 0; i < SlotWidgets.Num(); ++i)
    {
        if (!SlotWidgets[i])
        {
            continue;
        }

        // Get the SizeBox wrapper (parent of the slot widget)
        USizeBox* SizeBoxWrapper = Cast<USizeBox>(SlotWidgets[i]->GetParent());
        if (SizeBoxWrapper)
        {
            // Update SizeBox dimensions in case SlotSize changed
            SizeBoxWrapper->SetWidthOverride(SlotSize);
            SizeBoxWrapper->SetHeightOverride(SlotSize);

            if (UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(SizeBoxWrapper->Slot))
            {
                int32 Row = i / CalculatedColumns;
                int32 Col = i % CalculatedColumns;
                GridSlot->SetRow(Row);
                GridSlot->SetColumn(Col);
                GridSlot->SetHorizontalAlignment(HAlign_Center);
                GridSlot->SetVerticalAlignment(VAlign_Center);
            }
        }
    }
}

void URInventoryGridWidget::UpdateSlots()
{
    const TArray<FRInventoryEntry>* Entries = nullptr;
    int32 ItemCount = 0;

    if (Inventory)
    {
        Entries = &Inventory->GetItems();
        ItemCount = Entries->Num();
    }

    for (int32 i = 0; i < SlotWidgets.Num(); ++i)
    {
        if (!SlotWidgets[i])
        {
            continue;
        }

        if (Entries && i < ItemCount && (*Entries)[i].IsValidEntry())
        {
            SlotWidgets[i]->SetupFromEntry((*Entries)[i], i);
        }
        else
        {
            SlotWidgets[i]->SetupAsEmpty(i);
        }
    }
}

void URInventoryGridWidget::CreateEditorPreview()
{
    if (!GridPanel || !SlotWidgetClass)
    {
        return;
    }

    // Configure grid panel spacing
    GridPanel->SetSlotPadding(FMargin(SlotSpacing * 0.5f));
    GridPanel->SetMinDesiredSlotWidth(SlotSize);
    GridPanel->SetMinDesiredSlotHeight(SlotSize);

    // Clear existing
    GridPanel->ClearChildren();
    SlotWidgets.Empty();

    // Calculate slots needed for preview
    int32 RowsNeeded = (EditorPreviewItemCount + CalculatedColumns - 1) / CalculatedColumns;
    RowsNeeded = FMath::Max(RowsNeeded, MinVisibleRows);
    RowsNeeded = FMath::Max(RowsNeeded, 1);
    int32 TotalSlots = RowsNeeded * CalculatedColumns;

    for (int32 i = 0; i < TotalSlots; ++i)
    {
        URInventorySlotWidget* SlotWidget = CreateWidget<URInventorySlotWidget>(this, SlotWidgetClass);
        if (!SlotWidget)
        {
            continue;
        }

        // Calculate grid position
        int32 Row = i / CalculatedColumns;
        int32 Col = i % CalculatedColumns;

        // Create SizeBox wrapper to enforce square dimensions
        USizeBox* SizeBoxWrapper = NewObject<USizeBox>(this);
        SizeBoxWrapper->SetWidthOverride(SlotSize);
        SizeBoxWrapper->SetHeightOverride(SlotSize);

        // Add slot to SizeBox
        SizeBoxWrapper->AddChild(SlotWidget);

        // Add SizeBox to grid
        UUniformGridSlot* GridSlot = GridPanel->AddChildToUniformGrid(SizeBoxWrapper, Row, Col);
        if (GridSlot)
        {
            GridSlot->SetHorizontalAlignment(HAlign_Center);
            GridSlot->SetVerticalAlignment(VAlign_Center);
        }

        // Set up as preview item or empty
        if (i < EditorPreviewItemCount)
        {
            SlotWidget->SetupAsEditorPreview(i, 99);
        }
        else
        {
            SlotWidget->SetupAsEmpty(i);
        }

        SlotWidgets.Add(SlotWidget);
    }
}

void URInventoryGridWidget::HandleInventoryChanged(URInventoryComponent* ChangedInventory)
{
    if (ChangedInventory == Inventory)
    {
        RefreshSlots();
    }
}

void URInventoryGridWidget::HandleSlotClicked(URInventorySlotWidget* ClickedSlot, const FPointerEvent& PointerEvent)
{
    if (!ClickedSlot)
    {
        return;
    }

    int32 SlotIndex = ClickedSlot->GetSlotIndex();
    URItemDefinition* ItemDef = ClickedSlot->GetItemDefinition();

    OnSlotClicked.Broadcast(SlotIndex, ItemDef);
}

void URInventoryGridWidget::SetSort(ERInventorySortKey InKey, ERInventorySortDirection InDirection)
{
    SortKey = InKey;
    SortDirection = InDirection;

    if (Inventory)
    {
        Inventory->SortInventory(SortKey, SortDirection);
    }
}

void URInventoryGridWidget::ToggleSortDirection()
{
    SortDirection = (SortDirection == ERInventorySortDirection::Ascending)
                        ? ERInventorySortDirection::Descending
                        : ERInventorySortDirection::Ascending;

    if (Inventory)
    {
        Inventory->SortInventory(SortKey, SortDirection);
    }
}

URInventorySlotWidget* URInventoryGridWidget::GetSlotAt(int32 Index) const
{
    if (Index >= 0 && Index < SlotWidgets.Num())
    {
        return SlotWidgets[Index];
    }
    return nullptr;
}

int32 URInventoryGridWidget::GetCurrentRowCount() const
{
    if (CalculatedColumns <= 0)
    {
        return 0;
    }
    return (SlotWidgets.Num() + CalculatedColumns - 1) / CalculatedColumns;
}

int32 URInventoryGridWidget::GetItemCount() const
{
    if (!Inventory)
    {
        return 0;
    }

    int32 Count = 0;
    for (const FRInventoryEntry& Entry : Inventory->GetItems())
    {
        if (Entry.IsValidEntry())
        {
            Count++;
        }
    }
    return Count;
}

void URInventoryGridWidget::SwapSlots(int32 IndexA, int32 IndexB)
{
    if (Inventory)
    {
        Inventory->SwapEntries(IndexA, IndexB);
        OnItemSwapped.Broadcast(IndexA, IndexB, nullptr);
    }
}

void URInventoryGridWidget::MoveToSlot(int32 FromIndex, int32 ToIndex)
{
    if (Inventory)
    {
        Inventory->MoveEntry(FromIndex, ToIndex);
    }
}
