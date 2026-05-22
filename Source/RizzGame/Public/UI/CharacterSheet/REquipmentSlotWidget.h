// REquipmentSlotWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/RItemType.h"
#include "REquipmentSlotWidget.generated.h"

class UREquipmentComponent;
class URItemDefinition;
class UBorder;
class UImage;
class UTextBlock;
class UButton;
class USizeBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentSlotClicked, const FPointerEvent&, PointerEvent);

/**
 * Widget representing a single equipment slot on the character paper doll.
 * Supports drag-and-drop for equipping/unequipping items.
 */
UCLASS()
class RIZZGAME_API UREquipmentSlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UREquipmentSlotWidget(const FObjectInitializer& ObjectInitializer);

    /* ───────── Slot Configuration ───────── */

    /** The equipment slot this widget represents. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot")
    EREquipmentSlot SlotType = EREquipmentSlot::None;

    /** Size of the slot (square: width = height). Set to 0 to not override size. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot", meta = (ClampMin = "0", UIMin = "0"))
    float SlotSize = 64.0f;

    /** Icon to display when the slot is empty. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot")
    FSlateBrush EmptySlotIcon;

    /** Icon to display for editor preview. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot")
    FSlateBrush PreviewItemIcon;

    /** Border brushes for different item rarities. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot")
    TMap<ERItemRarity, FSlateBrush> RarityBorderBrushes;

    /** Default border brush when slot is empty or no rarity mapping exists. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot")
    FSlateBrush DefaultBorderBrush;

    /** Border brush for selected/toggled state. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot")
    FSlateBrush SelectedBorderBrush;

    /* ───────── Editor Preview ───────── */

    /** Enable preview in editor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot|Editor Preview")
    bool bShowEditorPreview = true;

    /** Show slot as filled in editor preview. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot|Editor Preview", meta = (EditCondition = "bShowEditorPreview"))
    bool bEditorPreviewFilled = false;

    /* ───────── Data Binding ───────── */

    /**
     * Bind this slot to an equipment component.
     * Will automatically update when equipment changes.
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment Slot")
    void BindToEquipmentComponent(UREquipmentComponent* InEquipmentComponent);

    /**
     * Unbind from the current equipment component.
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment Slot")
    void UnbindFromEquipmentComponent();

    /**
     * Manually set the item displayed in this slot.
     * Use this when not bound to an equipment component.
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment Slot")
    void SetItem(URItemDefinition* InItemDef);

    /**
     * Clear the slot to show empty state.
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment Slot")
    void ClearSlot();

    /**
     * Alias for ClearSlot for consistency with other slot widgets.
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment Slot")
    void ClearItem() { ClearSlot(); }

    /**
     * Set up as editor preview (shows preview icon).
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment Slot")
    void SetupAsEditorPreview();

    /**
     * Get the currently displayed item.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment Slot")
    URItemDefinition* GetCurrentItem() const { return CurrentItemDef; }

    /**
     * Check if the slot is currently empty.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment Slot")
    bool IsEmpty() const { return CurrentItemDef == nullptr && !bIsEditorPreview; }

    /* ───────── Selection State ───────── */

    /**
     * Set the selected/toggled state.
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment Slot")
    void SetToggled(bool bInToggled, bool bBroadcast = true);

    /**
     * Check if slot is toggled/selected.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment Slot")
    bool IsToggled() const { return bIsToggled; }

    /* ───────── Events ───────── */

    UPROPERTY(BlueprintAssignable, Category = "Equipment Slot|Events")
    FOnEquipmentSlotClicked OnClicked;

    /* ───────── Bound Widgets ───────── */

    /** The item icon image. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Equipment Slot|Bind")
    UImage* IMG_Icon = nullptr;

    /** Optional rarity border around the item. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Equipment Slot|Bind")
    UBorder* RarityBorder = nullptr;

    /** Text showing slot type label when empty. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Equipment Slot|Bind")
    UTextBlock* Txt_SlotLabel = nullptr;

    /** Optional button for click handling. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Equipment Slot|Bind")
    UButton* Btn_Slot = nullptr;

    /** Optional size box wrapper to enforce slot dimensions. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Equipment Slot|Bind")
    USizeBox* SizeBoxRoot = nullptr;

protected:
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    /** Update the visual display based on current item. */
    void UpdateVisualState();

    /** Apply rarity border color. */
    void ApplyRarityBorder();

    /** Set the icon brush. */
    void SetIconBrush(const FSlateBrush& NewBrush);

    /** Get display name for empty slot. */
    FText GetSlotDisplayName() const;

    /** Apply slot size to SizeBox if available. */
    void ApplySlotSize();

private:
    UFUNCTION()
    void HandleEquipmentChanged(EREquipmentSlot ChangedSlot, URItemDefinition* NewItem);

    UFUNCTION()
    void HandleButtonClicked();

    UPROPERTY()
    TObjectPtr<UREquipmentComponent> BoundEquipmentComponent = nullptr;

    UPROPERTY()
    TObjectPtr<URItemDefinition> CurrentItemDef = nullptr;

    UPROPERTY()
    bool bIsToggled = false;

    UPROPERTY()
    bool bIsEditorPreview = false;

    /** Stored pointer event for click broadcast. */
    FPointerEvent LastPointerEvent;
};
