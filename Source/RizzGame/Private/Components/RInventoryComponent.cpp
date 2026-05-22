// RInventoryComponent.cpp

#include "Components/RInventoryComponent.h" 
#include "Items/RItemDefinition.h"

URInventoryComponent::URInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
}

float URInventoryComponent::GetTotalWeight() const
{
    float Total = 0.f;
    for (const FRInventoryEntry& Entry : Entries)
    {
        if (Entry.ItemDefinition && Entry.Quantity > 0)
        {
            Total += Entry.ItemDefinition->GetWeightPerUnit() * Entry.Quantity;
        }
    }
    return Total;
}

int32 URInventoryComponent::GetTotalQuantityOfDefinition(URItemDefinition* ItemDef) const
{
    if (!ItemDef)
    {
        return 0;
    }

    int32 Total = 0;
    for (const FRInventoryEntry& Entry : Entries)
    {
        if (Entry.ItemDefinition == ItemDef)
        {
            Total += Entry.Quantity;
        }
    }
    return Total;
}

bool URInventoryComponent::HasItem(URItemDefinition* ItemDef, int32 RequiredQuantity) const
{
    if (!ItemDef || RequiredQuantity <= 0)
    {
        return false;
    }

    return GetTotalQuantityOfDefinition(ItemDef) >= RequiredQuantity;
}

int32 URInventoryComponent::RemoveQuantityOfDefinition(URItemDefinition* ItemDef, int32 Quantity)
{
    if (!ItemDef || Quantity <= 0)
    {
        return 0;
    }

    int32 Remaining = Quantity;

    for (int32 i = Entries.Num() - 1; i >= 0 && Remaining > 0; --i)
    {
        FRInventoryEntry& Entry = Entries[i];
        if (Entry.ItemDefinition != ItemDef)
        {
            continue;
        }

        const int32 ToRemove = FMath::Min(Entry.Quantity, Remaining);
        Entry.Quantity -= ToRemove;
        Remaining     -= ToRemove;

        if (Entry.Quantity <= 0)
        {
            Entries.RemoveAt(i);
        }
    }

    const int32 ActuallyRemoved = Quantity - Remaining;
    if (ActuallyRemoved > 0)
    {
        BroadcastInventoryChanged();
    }

    return ActuallyRemoved;
}

float URInventoryComponent::ComputeWeightWithAdditional(URItemDefinition* ItemDef, int32 Quantity) const
{
    if (!ItemDef || Quantity <= 0)
    {
        return GetTotalWeight();
    }

    const float Additional = ItemDef->GetWeightPerUnit() * Quantity;
    return GetTotalWeight() + Additional;
}

bool URInventoryComponent::CanFitItem(URItemDefinition* ItemDef, int32 Quantity) const
{
    if (!ItemDef || Quantity <= 0)
    {
        return false;
    }

    // Weight limit check
    if (LimitType == ERInventoryLimitType::Weight)
    {
        const float NewWeight = ComputeWeightWithAdditional(ItemDef, Quantity);
        if (NewWeight > MaxWeight)
        {
            return false;
        }
    }

    // Slot / stack check
    if (LimitType == ERInventoryLimitType::SlotCount)
    {
        const bool bStackable = ItemDef->IsStackable();
        const int32 MaxStack = bStackable ? FMath::Max(1, ItemDef->MaxStackSize) : 1;

        int32 FreeStackSpace = 0;
        int32 ExistingStacks = 0;

        for (const FRInventoryEntry& Entry : Entries)
        {
            if (Entry.ItemDefinition == ItemDef)
            {
                ++ExistingStacks;

                if (bStackable)
                {
                    const int32 Space = MaxStack - Entry.Quantity;
                    if (Space > 0)
                    {
                        FreeStackSpace += Space;
                    }
                }
            }
        }

        int32 Remaining = Quantity;

        if (bStackable)
        {
            if (FreeStackSpace >= Remaining)
            {
                // No new stacks needed.
                return true;
            }

            Remaining -= FreeStackSpace;
            const int32 NewStacksNeeded = FMath::DivideAndRoundUp(Remaining, MaxStack);
            const int32 TotalStacksAfter = Entries.Num() + NewStacksNeeded;

            return TotalStacksAfter <= MaxSlots;
        }
        else
        {
            // Non-stackable: 1 item = 1 stack
            const int32 NewStacksNeeded = Quantity;
            const int32 TotalStacksAfter = Entries.Num() + NewStacksNeeded;
            return TotalStacksAfter <= MaxSlots;
        }
    }

    // No limits or weight-only (already checked)
    return true;
}

bool URInventoryComponent::TryAddItem(URItemDefinition* ItemDef, int32 Quantity, int32 ChargesPerItem)
{
    if (!ItemDef || Quantity <= 0)
    {
        return false;
    }

    // All-or-nothing capacity check
    if (!CanFitItem(ItemDef, Quantity))
    {
        return false;
    }

    const bool bStackable = ItemDef->IsStackable();
    const int32 MaxStack = bStackable ? FMath::Max(1, ItemDef->MaxStackSize) : 1;

    int32 Remaining = Quantity;

    // First, fill existing stacks (if stackable)
    if (bStackable)
    {
        for (FRInventoryEntry& Entry : Entries)
        {
            if (Entry.ItemDefinition != ItemDef || Entry.Quantity >= MaxStack)
            {
                continue;
            }

            const int32 Space = MaxStack - Entry.Quantity;
            const int32 ToAdd = FMath::Min(Space, Remaining);

            Entry.Quantity += ToAdd;

            Remaining -= ToAdd;
            if (Remaining <= 0)
            {
                break;
            }
        }
    }
 
    while (Remaining > 0)
    {
        const int32 StackAmount = bStackable ? FMath::Min(MaxStack, Remaining) : 1;

        FRInventoryEntry NewEntry;
        NewEntry.ItemDefinition = ItemDef;
        NewEntry.Quantity       = StackAmount; 
 
        LastAddedSequence++;
        NewEntry.AddedSequence = LastAddedSequence;

        Entries.Add(NewEntry);

        Remaining -= StackAmount;
    }

    OnItemAdded.Broadcast(this, ItemDef, Quantity);
    BroadcastInventoryChanged();
    return true;
}


bool URInventoryComponent::RemoveFromEntry(int32 EntryIndex, int32 Quantity)
{
    if (!Entries.IsValidIndex(EntryIndex) || Quantity <= 0)
    {
        return false;
    }

    FRInventoryEntry& Entry = Entries[EntryIndex];
    if (Entry.Quantity <= 0)
    {
        return false;
    }

    const int32 RemoveAmount = FMath::Min(Quantity, Entry.Quantity);
    Entry.Quantity -= RemoveAmount;

    if (Entry.Quantity <= 0)
    {
        Entries.RemoveAt(EntryIndex);
    }

    BroadcastInventoryChanged();
    return true;
}

int32 URInventoryComponent::RemoveAllOfDefinition(URItemDefinition* ItemDef)
{
    if (!ItemDef)
    {
        return 0;
    }

    int32 RemovedTotal = 0;

    for (int32 i = Entries.Num() - 1; i >= 0; --i)
    {
        FRInventoryEntry& Entry = Entries[i];
        if (Entry.ItemDefinition == ItemDef)
        {
            RemovedTotal += Entry.Quantity;
            Entries.RemoveAt(i);
        }
    }

    if (RemovedTotal > 0)
    {
        BroadcastInventoryChanged();
    }

    return RemovedTotal;
}

void URInventoryComponent::ClearInventory()
{
    if (Entries.Num() == 0)
    {
        return;
    }

    Entries.Reset();
    BroadcastInventoryChanged();
}
void URInventoryComponent::SortInventory(ERInventorySortKey Key,ERInventorySortDirection Direction)
{
     if (Entries.Num() <= 1)
    {
        return;
    }

    const bool bAscending = (Direction == ERInventorySortDirection::Ascending);

    Entries.Sort([Key, bAscending](const FRInventoryEntry& A, const FRInventoryEntry& B)
    {
        auto CompareInt = [bAscending](int32 LA, int32 LB)
        {
            if (LA == LB)
            {
                return false;
            }
            return bAscending ? (LA < LB) : (LA > LB);
        };

        auto CompareFloat = [bAscending](float LA, float LB)
        {
            if (FMath::IsNearlyEqual(LA, LB))
            {
                return false;
            }
            return bAscending ? (LA < LB) : (LA > LB);
        };

        auto CompareString = [bAscending](const FString& LA, const FString& LB)
        {
            const int32 Res = LA.Compare(LB, ESearchCase::IgnoreCase);
            if (Res == 0)
            {
                return false;
            }
            return bAscending ? (Res < 0) : (Res > 0);
        };

        switch (Key)
        {
        case ERInventorySortKey::Name:
            {
                const FString AN = A.ItemDefinition ? A.ItemDefinition->GetName() : FString();
                const FString BN = B.ItemDefinition ? B.ItemDefinition->GetName() : FString();
                return CompareString(AN, BN);
            }

        case ERInventorySortKey::Type:
            {
                uint8 AT = 255;
                uint8 BT = 255;

                if (A.ItemDefinition)
                {
                    AT = static_cast<uint8>(A.ItemDefinition->ItemType);
                }
                if (B.ItemDefinition)
                {
                    BT = static_cast<uint8>(B.ItemDefinition->ItemType);
                }
                return CompareInt(AT, BT);
            }

        case ERInventorySortKey::Weight:
            { 
                const float AW = (A.ItemDefinition ? A.ItemDefinition->GetWeightPerUnit() : 0.f);
                const float BW = (B.ItemDefinition ? B.ItemDefinition->GetWeightPerUnit() : 0.f);
                return CompareFloat(AW, BW);
            }

        case ERInventorySortKey::Latest:
            { 
                const int32 AI = A.AddedSequence;
                const int32 BI = B.AddedSequence;
                return CompareInt(AI, BI);
            }

        default:
            return false;
        }
    });

    BroadcastInventoryChanged();
}
void URInventoryComponent::MoveEntry(int32 FromIndex,int32 ToIndex)
{
    if (FromIndex == ToIndex)
    {
        return;
    }

    if (!Entries.IsValidIndex(FromIndex))
    {
        return;
    }
 
    ToIndex = FMath::Clamp(ToIndex, 0, Entries.Num() - 1);

    FRInventoryEntry Temp = Entries[FromIndex];
    Entries.RemoveAt(FromIndex);
 
    if (ToIndex > FromIndex)
    {
        ToIndex -= 1;
    }

    ToIndex = FMath::Clamp(ToIndex, 0, Entries.Num());
    Entries.Insert(Temp, ToIndex);

    BroadcastInventoryChanged();
}
void URInventoryComponent::SwapEntries(int32 IndexA,int32 IndexB)
{
    if (IndexA == IndexB)
    {
        return;
    }

    if (!Entries.IsValidIndex(IndexA) || !Entries.IsValidIndex(IndexB))
    {
        return;
    }

    Entries.Swap(IndexA, IndexB);
    BroadcastInventoryChanged();
}

void URInventoryComponent::BroadcastInventoryChanged()
{
    OnInventoryChanged.Broadcast(this);
}
