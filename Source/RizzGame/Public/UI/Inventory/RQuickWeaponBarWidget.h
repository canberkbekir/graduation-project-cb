// RQuickWeaponBarWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RQuickWeaponBarWidget.generated.h"

class UREquipmentComponent;
class UREquipmentSlotWidget;
class URItemDefinition;
class UPanelWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuickSlotSelected, int32, SlotIndex, URItemDefinition*, ItemDef);

/**
 * Widget displaying quick weapon slots for fast weapon switching.
 * Slots are placed manually in Blueprint - this widget discovers and manages them.
 * Binds to UREquipmentComponent for weapon management.
 */
UCLASS()
class RIZZGAME_API URQuickWeaponBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /* ───────── Editor Preview ───────── */

    /** Enable preview in editor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quick Weapon Bar|Editor Preview")
    bool bShowEditorPreview = true;

    /** Number of slots to show as "filled" in preview. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quick Weapon Bar|Editor Preview", meta = (ClampMin = "0", EditCondition = "bShowEditorPreview"))
    int32 EditorPreviewFilledSlots = 2;

    /** Which slot index to show as "selected" in preview. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quick Weapon Bar|Editor Preview", meta = (ClampMin = "-1", EditCondition = "bShowEditorPreview"))
    int32 EditorPreviewSelectedIndex = 0;

    /* ───────── Initialization ───────── */

    /**
     * Initialize the weapon bar with an equipment component.
     */
    UFUNCTION(BlueprintCallable, Category = "Quick Weapon Bar")
    void Init(UREquipmentComponent* InEquipmentComponent);

    /**
     * Refresh the weapon bar display from equipment component.
     */
    UFUNCTION(BlueprintCallable, Category = "Quick Weapon Bar")
    void Refresh();

    /**
     * Manually discover and register slot widgets from the container.
     * Call this if you add slots dynamically at runtime.
     */
    UFUNCTION(BlueprintCallable, Category = "Quick Weapon Bar")
    void DiscoverSlots();

    /* ───────── Slot Access ───────── */

    /**
     * Get the number of discovered slots.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quick Weapon Bar")
    int32 GetSlotCount() const { return SlotWidgets.Num(); }

    /**
     * Get the slot widget at a specific index.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quick Weapon Bar")
    UREquipmentSlotWidget* GetSlotAt(int32 Index) const;

    /**
     * Set the item in a quick slot.
     */
    UFUNCTION(BlueprintCallable, Category = "Quick Weapon Bar")
    void SetQuickSlot(int32 SlotIndex, URItemDefinition* ItemDef);

    /**
     * Equip the weapon from a quick slot.
     */
    UFUNCTION(BlueprintCallable, Category = "Quick Weapon Bar")
    void EquipFromSlot(int32 SlotIndex);

    /**
     * Get the currently selected quick slot index.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quick Weapon Bar")
    int32 GetSelectedSlotIndex() const { return SelectedSlotIndex; }

    /**
     * Select a quick slot (visual highlight).
     */
    UFUNCTION(BlueprintCallable, Category = "Quick Weapon Bar")
    void SelectSlot(int32 SlotIndex);

    /* ───────── Events ───────── */

    UPROPERTY(BlueprintAssignable, Category = "Quick Weapon Bar|Events")
    FOnQuickSlotSelected OnQuickSlotSelected;

    /* ───────── Bound Widgets ───────── */

    /**
     * Optional container holding the slot widgets.
     * If bound, slots will be discovered from its children.
     * Can be HorizontalBox, VerticalBox, GridPanel, or any PanelWidget.
     */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Quick Weapon Bar|Bind")
    UPanelWidget* SlotsContainer = nullptr;

protected:
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** Set up editor preview state on slots. */
    void CreateEditorPreview();

    /** Bind click events to all discovered slots. */
    void BindSlotEvents();

    /** Unbind click events from all slots. */
    void UnbindSlotEvents();

    /** Handle slot click. */
    UFUNCTION()
    void HandleSlotClicked(const FPointerEvent& PointerEvent);

private:
    UPROPERTY()
    TObjectPtr<UREquipmentComponent> EquipmentComponent = nullptr;

    UPROPERTY()
    TArray<TObjectPtr<UREquipmentSlotWidget>> SlotWidgets;

    UPROPERTY()
    int32 SelectedSlotIndex = 0;
};
