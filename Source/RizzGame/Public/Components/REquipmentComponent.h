// REquipmentComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/RItemType.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "REquipmentComponent.generated.h"

class URItemDefinition;
class URInventoryComponent;
class UAbilitySystemComponent;

/**
 * Represents an item equipped in a specific slot.
 * Tracks the item definition and any active GAS effects applied by the item.
 */
USTRUCT(BlueprintType)
struct FREquippedItem
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    EREquipmentSlot Slot = EREquipmentSlot::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    URItemDefinition* ItemDef = nullptr;

    UPROPERTY()
    TArray<FActiveGameplayEffectHandle> ActiveEffects;

    UPROPERTY()
    TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

    bool IsValid() const { return ItemDef != nullptr && Slot != EREquipmentSlot::None; }

    void Clear()
    {
        Slot = EREquipmentSlot::None;
        ItemDef = nullptr;
        ActiveEffects.Empty();
        GrantedAbilityHandles.Empty();
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentChanged, EREquipmentSlot, Slot, URItemDefinition*, NewItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentSlotCleared, EREquipmentSlot, Slot);

/**
 * Component that manages equipped items on a character.
 * Handles equipping/unequipping items, applying/removing GAS effects, and syncing with inventory.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RIZZGAME_API UREquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UREquipmentComponent();

    /* ───────── Equipment API ───────── */

    /**
     * Attempt to equip an item in the specified slot.
     * If an item is already equipped in that slot, it will be unequipped first.
     * @param ItemDef The item definition to equip
     * @param TargetSlot The slot to equip the item in
     * @param bRemoveFromInventory If true, removes the item from linked inventory
     * @return True if the item was successfully equipped
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool EquipItem(URItemDefinition* ItemDef, EREquipmentSlot TargetSlot, bool bRemoveFromInventory = true);

    /**
     * Unequip the item in the specified slot.
     * @param Slot The slot to unequip
     * @param bReturnToInventory If true, returns the item to the linked inventory
     * @return True if an item was successfully unequipped
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool UnequipSlot(EREquipmentSlot Slot, bool bReturnToInventory = true);

    /**
     * Get the item definition currently equipped in a slot.
     * @param Slot The slot to query
     * @return The item definition, or nullptr if the slot is empty
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
    URItemDefinition* GetEquippedItem(EREquipmentSlot Slot) const;

    /**
     * Get the full equipped item data for a slot.
     * @param Slot The slot to query
     * @param OutEquippedItem Output parameter for the equipped item data
     * @return True if the slot has an equipped item
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
    bool GetEquippedItemData(EREquipmentSlot Slot, FREquippedItem& OutEquippedItem) const;

    /**
     * Check if a slot is currently occupied.
     * @param Slot The slot to check
     * @return True if an item is equipped in the slot
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
    bool IsSlotOccupied(EREquipmentSlot Slot) const;

    /**
     * Get all currently equipped items.
     * @return Map of slot to equipped item data
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
    TMap<EREquipmentSlot, FREquippedItem> GetAllEquippedItems() const { return EquippedItems; }

    /**
     * Unequip all items.
     * @param bReturnToInventory If true, returns all items to inventory
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void UnequipAll(bool bReturnToInventory = true);

    /* ───────── Quick Weapon Slots ───────── */

    /** Quick weapon slot bindings (1-4) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|QuickSlots")
    TArray<URItemDefinition*> QuickWeaponSlots;

    UFUNCTION(BlueprintCallable, Category = "Equipment|QuickSlots")
    void SetQuickWeaponSlot(int32 SlotIndex, URItemDefinition* ItemDef);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment|QuickSlots")
    URItemDefinition* GetQuickWeaponSlot(int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Equipment|QuickSlots")
    void EquipFromQuickSlot(int32 SlotIndex);

    /* ───────── Inventory Link ───────── */

    UPROPERTY(BlueprintReadWrite, Category = "Equipment")
    TObjectPtr<URInventoryComponent> LinkedInventory;

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void SetLinkedInventory(URInventoryComponent* Inventory) { LinkedInventory = Inventory; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
    URInventoryComponent* GetLinkedInventory() const { return LinkedInventory; }

    /* ───────── Events ───────── */

    UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
    FOnEquipmentChanged OnEquipmentChanged;

    UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
    FOnEquipmentSlotCleared OnEquipmentSlotCleared;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    TMap<EREquipmentSlot, FREquippedItem> EquippedItems;

private:
    void ApplyEquipEffects(URItemDefinition* ItemDef, FREquippedItem& EquippedItem);
    void RemoveEquipEffects(FREquippedItem& EquippedItem);
    UAbilitySystemComponent* GetOwnerASC() const;

    static constexpr int32 MaxQuickWeaponSlots = 4;
};
