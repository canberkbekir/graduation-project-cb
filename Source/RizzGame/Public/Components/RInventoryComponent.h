// RInventoryComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RInventoryComponent.generated.h"

class URItemDefinition;
class URInventoryComponent;

/** How the inventory capacity is limited. */
UENUM(BlueprintType)
enum class ERInventoryLimitType : uint8
{
    None      UMETA(DisplayName = "No Limit"),
    SlotCount UMETA(DisplayName = "Slot Count"),
    Weight    UMETA(DisplayName = "Total Weight")
};

UENUM(BlueprintType)
enum class ERInventorySortKey : uint8
{
    Name    UMETA(DisplayName = "Name"),
    Type    UMETA(DisplayName = "Type"),
    Weight  UMETA(DisplayName = "Weight"),
    Latest  UMETA(DisplayName = "Latest")
};

UENUM(BlueprintType)
enum class ERInventorySortDirection : uint8
{
    Ascending  UMETA(DisplayName = "Ascending"),
    Descending UMETA(DisplayName = "Descending")
};

/**
 * One logical stack / entry inside the inventory.
 * Holds a reference to the item definition asset and quantity.
 */
USTRUCT(BlueprintType)
struct FRInventoryEntry
{
    GENERATED_BODY()

    /** Item definition (Primary Data Asset). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    URItemDefinition* ItemDefinition = nullptr;

    /** How many items are in this stack. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "0", UIMin = "0"))
    int32 Quantity = 0;

    /** Monotonic index used for 'Latest' sorting (higher = newer). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 AddedSequence = 0;

    /** Convenience: used by UI to check if slot has a real item. */
    bool IsValidEntry() const
    {
        return ItemDefinition != nullptr && Quantity > 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, URInventoryComponent*, Inventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemAdded, URInventoryComponent*, Inventory, URItemDefinition*, ItemDef, int32, Quantity);
 
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RIZZGAME_API URInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URInventoryComponent();

    /* ───────── Settings ───────── */

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    ERInventoryLimitType LimitType = ERInventoryLimitType::None;

    // Max number of stacks if LimitType == SlotCount
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory",
        meta = (EditCondition = "LimitType == ERInventoryLimitType::SlotCount", ClampMin = "1", UIMin = "1"))
    int32 MaxSlots = 30;

    // Max total weight if LimitType == Weight
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory",
        meta = (EditCondition = "LimitType == ERInventoryLimitType::Weight", ClampMin = "0.0", UIMin = "0.0"))
    float MaxWeight = 100.f;

protected:
    // Stored stacks
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FRInventoryEntry> Entries;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 LastAddedSequence = 0;

public:
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;

    /** Fired once per TryAddItem call with the definition and total quantity added. */
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnItemAdded OnItemAdded;

    /* ───────── Query API ───────── */
 
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    TArray<FRInventoryEntry> GetItemsWithoutReference() const { return Entries; }
 
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    const TArray<FRInventoryEntry>& GetItems() const { return Entries; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    float GetTotalWeight() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    int32 GetTotalQuantityOfDefinition(URItemDefinition* ItemDef) const;

    /** Returns true if the inventory contains at least RequiredQuantity of ItemDef. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    bool HasItem(URItemDefinition* ItemDef, int32 RequiredQuantity = 1) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    int32 GetTotalSlotsUsed() const { return Entries.Num(); }
 
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    ERInventoryLimitType GetLimitType() const { return LimitType; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    int32 GetMaxSlots() const { return MaxSlots; }

    /* ───────── Mutation API ───────── */

    /**
     * Try to add a quantity of an item definition.
     * All-or-nothing: returns false if it cannot fully fit due to slot/weight limits.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAddItem(URItemDefinition* ItemDef, int32 Quantity, int32 ChargesPerItem = 0);

    /** Remove quantity from a specific entry index. Removes the stack if it reaches zero. */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveFromEntry(int32 EntryIndex, int32 Quantity);

    /** Remove all of a given definition; returns how many were removed. */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 RemoveAllOfDefinition(URItemDefinition* ItemDef);

    /**
     * Remove up to Quantity of ItemDef across all stacks (empties stacks as it goes).
     * Returns how many were actually removed — may be less than Quantity if not enough in inventory.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 RemoveQuantityOfDefinition(URItemDefinition* ItemDef, int32 Quantity);

    /** Clears the inventory completely. */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Sorting")
    void SortInventory(ERInventorySortKey Key, ERInventorySortDirection Direction);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Ordering")
    void MoveEntry(int32 FromIndex, int32 ToIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Ordering")
    void SwapEntries(int32 IndexA, int32 IndexB);

protected:
    virtual void BeginPlay() override;

private:
    bool CanFitItem(URItemDefinition* ItemDef, int32 Quantity) const;
    float ComputeWeightWithAdditional(URItemDefinition* ItemDef, int32 Quantity) const;
    void BroadcastInventoryChanged();
};
