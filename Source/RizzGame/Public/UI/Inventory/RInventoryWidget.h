// RInventoryWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/RInventoryComponent.h"
#include "RInventoryWidget.generated.h"

class UPanelWidget;
class URInventoryItemWidget;
class URInventoryGridWidget;
class URQuickWeaponBarWidget;
class UREquipmentComponent;
class UComboBoxString;

/**
 * Root inventory widget.
 * Binds to URInventoryComponent, displays items in a grid layout with quick weapon bar.
 */
UCLASS()
class RIZZGAME_API URInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /* ───────── Initialization ───────── */

    /**
     * Initialize with inventory and equipment components.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void Init(URInventoryComponent* InInventory, UREquipmentComponent* InEquipment = nullptr);

    /**
     * Legacy init method for backwards compatibility.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void InitLegacy(URInventoryComponent* InInventory);

    /* ───────── Widget References ───────── */

    /** Root panel that contains item widgets (legacy - for backwards compatibility). */
    UPROPERTY(meta = (BindWidgetOptional))
    UPanelWidget* ItemsRoot = nullptr;

    /** New grid-based inventory display. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory|Bind")
    URInventoryGridWidget* InventoryGrid = nullptr;

    /** Quick weapon bar for fast weapon switching. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory|Bind")
    URQuickWeaponBarWidget* QuickWeaponBar = nullptr;

    /** Sort dropdown for changing sort options. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory|Bind")
    UComboBoxString* CB_SortDropdown = nullptr;

    /** Widget class used for legacy list rows. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    TSubclassOf<URInventoryItemWidget> ItemWidgetClass;

    /** Bound inventory component. */
    UPROPERTY()
    URInventoryComponent* Inventory;

    /** Bound equipment component. */
    UPROPERTY()
    UREquipmentComponent* Equipment;

    /* ───────── Inventory Callbacks ───────── */

    /** Called when inventory changes. */
    UFUNCTION()
    void HandleInventoryChanged(URInventoryComponent* ChangedInventory);

    /** Rebuild the list of child widgets from inventory entries (legacy mode). */
    void RebuildItems();

    /* ───────── Sorting ───────── */

    /** Current sort key (Name, Type, Weight, Latest). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Sorting")
    ERInventorySortKey SortKey = ERInventorySortKey::Name;

    /** Current sort direction (Ascending / Descending). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Sorting")
    ERInventorySortDirection SortDirection = ERInventorySortDirection::Ascending;

    /** Called from UI buttons to set sort key + direction. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Sorting")
    void SetSort(ERInventorySortKey InKey, ERInventorySortDirection InDirection);

    /** Toggle between Asc / Desc for the current SortKey. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Sorting")
    void ToggleSortDirection();

    /* ───────── Item Operations ───────── */

    /**
     * Move an item from one index to another.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Ordering")
    void MoveItem(int32 FromIndex, int32 ToIndex);

    /**
     * Swap two items.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Ordering")
    void SwapItems(int32 IndexA, int32 IndexB);

    /* ───────── Animation Hooks ───────── */

    /**
     * Play the slide-in animation.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Animation")
    void PlaySlideInAnimation();

    /**
     * Play the slide-out animation.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Animation")
    void PlaySlideOutAnimation();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** Blueprint hook for slide-in animation. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Animation")
    void BP_PlaySlideInAnimation();

    /** Blueprint hook for slide-out animation. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Animation")
    void BP_PlaySlideOutAnimation();

    /** Set up sort dropdown options. */
    void SetupSortDropdown();

    /** Handle sort dropdown selection change. */
    UFUNCTION()
    void HandleSortDropdownChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
};
