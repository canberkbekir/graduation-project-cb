// RInventorySlotWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/RItemType.h"
#include "Components/RInventoryComponent.h"
#include "RInventorySlotWidget.generated.h"

class URItemDefinition;
class UBorder;
class UImage;
class UTextBlock;
class UButton;
class USizeBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventorySlotClicked, URInventorySlotWidget*, ClickedSlot, const FPointerEvent&, PointerEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventorySlotDragStarted, URInventorySlotWidget*, DraggedSlot);

/**
 * Widget representing a single inventory slot in a grid.
 * Supports drag-and-drop for item management.
 */
UCLASS()
class RIZZGAME_API URInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    URInventorySlotWidget(const FObjectInitializer& ObjectInitializer);

    /* ───────── Setup ───────── */

    /**
     * Set up the slot to display an inventory entry.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
    void SetupFromEntry(const FRInventoryEntry& Entry, int32 InSlotIndex);

    /**
     * Set up the slot as empty.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
    void SetupAsEmpty(int32 InSlotIndex);

    /**
     * Set up as editor preview slot with fake data.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
    void SetupAsEditorPreview(int32 InSlotIndex, int32 InPreviewQuantity = 99);

    /**
     * Get the slot index in the inventory grid.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Slot")
    int32 GetSlotIndex() const { return SlotIndex; }

    /**
     * Get the item definition in this slot.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Slot")
    URItemDefinition* GetItemDefinition() const { return CurrentEntry.ItemDefinition; }

    /**
     * Get the quantity displayed in this slot.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Slot")
    int32 GetQuantity() const { return CurrentEntry.Quantity; }

    /**
     * Check if the slot is empty.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Slot")
    bool IsSlotEmpty() const { return !CurrentEntry.IsValidEntry() && !bIsEditorPreview; }

    /**
     * Check if this is an editor preview slot.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Slot")
    bool IsEditorPreview() const { return bIsEditorPreview; }

    /* ───────── Selection State ───────── */

    /**
     * Set the selected/toggled state.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
    void SetSelected(bool bInSelected);

    /**
     * Check if slot is selected.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Slot")
    bool IsSelected() const { return bIsSelected; }

    /* ───────── Size Configuration ───────── */

    /** Size of the slot (square: width = height). Set to 0 to not override size. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot", meta = (ClampMin = "0", UIMin = "0"))
    float SlotSize = 64.0f;

    /* ───────── Editor Preview ───────── */

    /** Enable preview in editor (for standalone use). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot|Editor Preview")
    bool bShowEditorPreview = false;

    /** Quantity to show in standalone editor preview. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot|Editor Preview", meta = (ClampMin = "0", EditCondition = "bShowEditorPreview"))
    int32 EditorPreviewQuantity = 99;

    /* ───────── Visual Configuration ───────── */

    /** Border brushes for different item rarities. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot|Visuals")
    TMap<ERItemRarity, FSlateBrush> RarityBorderBrushes;

    /** Default border brush for empty slots or unmapped rarities. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot|Visuals")
    FSlateBrush DefaultBorderBrush;

    /** Icon to display when the slot is empty. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot|Visuals")
    FSlateBrush EmptySlotIcon;

    /** Icon to display for editor preview items. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot|Visuals")
    FSlateBrush PreviewItemIcon;

    /** Minimum quantity to show the quantity text (e.g., 2 to hide "1"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot|Visuals")
    int32 MinQuantityToShow = 2;

    /** Background brush for selected state. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot|Visuals")
    FSlateBrush SelectedBorderBrush;

    /* ───────── Bound Widgets ───────── */

    /** The item icon image. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory Slot|Bind")
    UImage* IMG_Icon = nullptr;

    /** Text displaying the item quantity. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory Slot|Bind")
    UTextBlock* Txt_Quantity = nullptr;

    /** Border for showing item rarity. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory Slot|Bind")
    UBorder* RarityBorder = nullptr;

    /** Optional text for item name (usually hidden in grid view). */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory Slot|Bind")
    UTextBlock* Txt_ItemName = nullptr;

    /** Optional button for click handling. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory Slot|Bind")
    UButton* Btn_Slot = nullptr;

    /** Optional size box wrapper to enforce slot dimensions. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory Slot|Bind")
    USizeBox* SizeBoxRoot = nullptr;

    /* ───────── Events ───────── */

    UPROPERTY(BlueprintAssignable, Category = "Inventory Slot|Events")
    FOnInventorySlotClicked OnSlotClicked;

    UPROPERTY(BlueprintAssignable, Category = "Inventory Slot|Events")
    FOnInventorySlotDragStarted OnDragStarted;

protected:
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    /** Update the visual display based on current entry. */
    void UpdateVisualState();

    /** Apply rarity border color. */
    void ApplyRarityBorder();

    /** Update quantity text visibility and value. */
    void UpdateQuantityDisplay();

    /** Set the icon brush. */
    void SetIconBrush(const FSlateBrush& NewBrush);

    /** Check if slot has displayable content (item or preview). */
    bool HasDisplayableContent() const;

    /** Apply slot size to SizeBox if available. */
    void ApplySlotSize();

private:
    UFUNCTION()
    void HandleButtonClicked();

    UPROPERTY()
    FRInventoryEntry CurrentEntry;

    UPROPERTY()
    int32 SlotIndex = -1;

    UPROPERTY()
    bool bIsEditorPreview = false;

    UPROPERTY()
    int32 PreviewQuantity = 0;

    UPROPERTY()
    bool bIsSelected = false;

    /** Stored pointer event for click broadcast. */
    FPointerEvent LastPointerEvent;
};
