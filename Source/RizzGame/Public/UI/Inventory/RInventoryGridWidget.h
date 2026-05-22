// RInventoryGridWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/RInventoryComponent.h"
#include "RInventoryGridWidget.generated.h"

class URInventorySlotWidget;
class UUniformGridPanel;
class USizeBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGridSlotClicked, int32, SlotIndex, URItemDefinition*, ItemDef);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGridItemSwapped, int32, FromIndex, int32, ToIndex, URItemDefinition*, ItemDef);

/**
 * Grid-based inventory widget that displays items in an auto-sizing grid layout.
 * Columns are calculated automatically based on available width, slot size, and spacing.
 * Uses URInventorySlotWidget for each cell.
 */
UCLASS()
class RIZZGAME_API URInventoryGridWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /* ───────── Grid Configuration ───────── */

    /** Size of each slot (square: width = height). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Grid", meta = (ClampMin = "16", UIMin = "16"))
    float SlotSize = 64.0f;

    /** Spacing between slots. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Grid", meta = (ClampMin = "0", UIMin = "0"))
    float SlotSpacing = 4.0f;

    /** Minimum number of visible rows (shows empty slots to fill). Set to 0 for no minimum. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Grid", meta = (ClampMin = "0", UIMin = "0"))
    int32 MinVisibleRows = 0;

    /** Widget class to use for each slot. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Grid")
    TSubclassOf<URInventorySlotWidget> SlotWidgetClass;

    /* ───────── Editor Preview ───────── */

    /** Enable preview slots in editor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Grid|Editor Preview")
    bool bShowEditorPreview = true;

    /** Number of sample items to show in editor preview. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Grid|Editor Preview", meta = (ClampMin = "0", UIMin = "0", EditCondition = "bShowEditorPreview"))
    int32 EditorPreviewItemCount = 7;

    /** Preview width to use in editor (simulates panel width). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Grid|Editor Preview", meta = (ClampMin = "100", UIMin = "100", EditCondition = "bShowEditorPreview"))
    float EditorPreviewWidth = 300.0f;

    /* ───────── Initialization ───────── */

    /**
     * Initialize the grid and bind to an inventory component.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Grid")
    void Init(URInventoryComponent* InInventory);

    /**
     * Rebuild the grid slots from the bound inventory.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Grid")
    void RebuildGrid();

    /**
     * Refresh slot contents without recreating widgets.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Grid")
    void RefreshSlots();

    /**
     * Force recalculate columns based on current width.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Grid")
    void RecalculateColumns();

    /**
     * Get the bound inventory component.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Grid")
    URInventoryComponent* GetInventory() const { return Inventory; }

    /**
     * Get the current calculated column count.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Grid")
    int32 GetColumnCount() const { return CalculatedColumns; }

    /* ───────── Sorting ───────── */

    /** Current sort key. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Grid|Sorting")
    ERInventorySortKey SortKey = ERInventorySortKey::Name;

    /** Current sort direction. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Grid|Sorting")
    ERInventorySortDirection SortDirection = ERInventorySortDirection::Ascending;

    /**
     * Set sort key and direction, then rebuild.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Grid|Sorting")
    void SetSort(ERInventorySortKey InKey, ERInventorySortDirection InDirection);

    /**
     * Toggle sort direction.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Grid|Sorting")
    void ToggleSortDirection();

    /* ───────── Slot Access ───────── */

    /**
     * Get the slot widget at a specific index.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Grid")
    URInventorySlotWidget* GetSlotAt(int32 Index) const;

    /**
     * Get the current number of slots (including empty visual slots).
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Grid")
    int32 GetTotalSlots() const { return SlotWidgets.Num(); }

    /**
     * Get the number of rows currently displayed.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Grid")
    int32 GetCurrentRowCount() const;

    /**
     * Get the number of items in inventory.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Grid")
    int32 GetItemCount() const;

    /* ───────── Item Operations ───────── */

    /**
     * Swap items between two slots.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Grid")
    void SwapSlots(int32 IndexA, int32 IndexB);

    /**
     * Move an item from one slot to another.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Grid")
    void MoveToSlot(int32 FromIndex, int32 ToIndex);

    /* ───────── Events ───────── */

    UPROPERTY(BlueprintAssignable, Category = "Inventory Grid|Events")
    FOnGridSlotClicked OnSlotClicked;

    UPROPERTY(BlueprintAssignable, Category = "Inventory Grid|Events")
    FOnGridItemSwapped OnItemSwapped;

    /* ───────── Bound Widgets ───────── */

    /** The grid panel that contains the slot widgets. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory Grid|Bind")
    UUniformGridPanel* GridPanel = nullptr;

protected:
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /** Handle inventory changes. */
    UFUNCTION()
    void HandleInventoryChanged(URInventoryComponent* ChangedInventory);

    /** Handle slot click events. */
    UFUNCTION()
    void HandleSlotClicked(URInventorySlotWidget* ClickedSlot, const FPointerEvent& PointerEvent);

    /** Calculate columns based on available width. */
    int32 CalculateColumnsForWidth(float AvailableWidth) const;

    /** Calculate required number of slots (items + empty to fill rows). */
    int32 CalculateRequiredSlots() const;

    /** Create or adjust slot widgets to match required count. */
    void EnsureSlotCount(int32 RequiredSlots);

    /** Update existing slots with inventory data. */
    void UpdateSlots();

    /** Update slot positions in the grid. */
    void UpdateSlotPositions();

    /** Create editor preview slots. */
    void CreateEditorPreview();

    /** Get current available width for the grid. */
    float GetAvailableWidth() const;

private:
    UPROPERTY()
    TObjectPtr<URInventoryComponent> Inventory = nullptr;

    UPROPERTY()
    TArray<TObjectPtr<URInventorySlotWidget>> SlotWidgets;

    UPROPERTY()
    int32 CalculatedColumns = 4;

    UPROPERTY()
    float LastKnownWidth = 0.0f;
};
