// REquipmentComponent.cpp

#include "Components/REquipmentComponent.h"
#include "Components/RInventoryComponent.h"
#include "Items/RItemDefinition.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

UREquipmentComponent::UREquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    QuickWeaponSlots.SetNum(MaxQuickWeaponSlots);
}

void UREquipmentComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool UREquipmentComponent::EquipItem(URItemDefinition* ItemDef, EREquipmentSlot TargetSlot, bool bRemoveFromInventory)
{
    if (!ItemDef || TargetSlot == EREquipmentSlot::None)
    {
        return false;
    }

    // If slot is occupied, unequip first
    if (IsSlotOccupied(TargetSlot))
    {
        UnequipSlot(TargetSlot, true);
    }

    // Remove from inventory if requested
    if (bRemoveFromInventory && LinkedInventory)
    {
        int32 RemovedCount = LinkedInventory->RemoveAllOfDefinition(ItemDef);
        if (RemovedCount == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("EquipItem: Item '%s' not found in inventory"), *ItemDef->GetName());
            // Continue anyway - item might be equipped from other sources
        }
    }

    // Create equipped item entry
    FREquippedItem EquippedItem;
    EquippedItem.Slot = TargetSlot;
    EquippedItem.ItemDef = ItemDef;

    // Apply GAS effects
    ApplyEquipEffects(ItemDef, EquippedItem);

    // Store in map
    EquippedItems.Add(TargetSlot, EquippedItem);

    // Broadcast change
    OnEquipmentChanged.Broadcast(TargetSlot, ItemDef);

    UE_LOG(LogTemp, Log, TEXT("Equipped '%s' in slot %d"), *ItemDef->Visual.ItemName.ToString(), static_cast<int32>(TargetSlot));

    return true;
}

bool UREquipmentComponent::UnequipSlot(EREquipmentSlot Slot, bool bReturnToInventory)
{
    if (Slot == EREquipmentSlot::None)
    {
        return false;
    }

    FREquippedItem* EquippedItem = EquippedItems.Find(Slot);
    if (!EquippedItem || !EquippedItem->IsValid())
    {
        return false;
    }

    URItemDefinition* ItemDef = EquippedItem->ItemDef;

    // Remove GAS effects
    RemoveEquipEffects(*EquippedItem);

    // Return to inventory if requested
    if (bReturnToInventory && LinkedInventory && ItemDef)
    {
        LinkedInventory->TryAddItem(ItemDef, 1);
    }

    // Remove from map
    EquippedItems.Remove(Slot);

    // Broadcast change
    OnEquipmentSlotCleared.Broadcast(Slot);
    OnEquipmentChanged.Broadcast(Slot, nullptr);

    UE_LOG(LogTemp, Log, TEXT("Unequipped item from slot %d"), static_cast<int32>(Slot));

    return true;
}

URItemDefinition* UREquipmentComponent::GetEquippedItem(EREquipmentSlot Slot) const
{
    const FREquippedItem* EquippedItem = EquippedItems.Find(Slot);
    return EquippedItem ? EquippedItem->ItemDef : nullptr;
}

bool UREquipmentComponent::GetEquippedItemData(EREquipmentSlot Slot, FREquippedItem& OutEquippedItem) const
{
    const FREquippedItem* EquippedItem = EquippedItems.Find(Slot);
    if (EquippedItem && EquippedItem->IsValid())
    {
        OutEquippedItem = *EquippedItem;
        return true;
    }
    return false;
}

bool UREquipmentComponent::IsSlotOccupied(EREquipmentSlot Slot) const
{
    const FREquippedItem* EquippedItem = EquippedItems.Find(Slot);
    return EquippedItem && EquippedItem->IsValid();
}

void UREquipmentComponent::UnequipAll(bool bReturnToInventory)
{
    TArray<EREquipmentSlot> SlotsToUnequip;
    EquippedItems.GetKeys(SlotsToUnequip);

    for (EREquipmentSlot Slot : SlotsToUnequip)
    {
        UnequipSlot(Slot, bReturnToInventory);
    }
}

void UREquipmentComponent::SetQuickWeaponSlot(int32 SlotIndex, URItemDefinition* ItemDef)
{
    if (SlotIndex >= 0 && SlotIndex < MaxQuickWeaponSlots)
    {
        QuickWeaponSlots[SlotIndex] = ItemDef;
    }
}

URItemDefinition* UREquipmentComponent::GetQuickWeaponSlot(int32 SlotIndex) const
{
    if (SlotIndex >= 0 && SlotIndex < QuickWeaponSlots.Num())
    {
        return QuickWeaponSlots[SlotIndex];
    }
    return nullptr;
}

void UREquipmentComponent::EquipFromQuickSlot(int32 SlotIndex)
{
    URItemDefinition* ItemDef = GetQuickWeaponSlot(SlotIndex);
    if (ItemDef)
    {
        // Determine target slot based on item type
        EREquipmentSlot TargetSlot = EREquipmentSlot::MainHand;
        if (ItemDef->Visual.DefaultSlot == ERItemSlot::WeaponOffHand || ItemDef->Visual.DefaultSlot == ERItemSlot::Shield)
        {
            TargetSlot = EREquipmentSlot::OffHand;
        }

        EquipItem(ItemDef, TargetSlot, true);
    }
}

void UREquipmentComponent::ApplyEquipEffects(URItemDefinition* ItemDef, FREquippedItem& EquippedItem)
{
    UAbilitySystemComponent* ASC = GetOwnerASC();
    if (!ASC || !ItemDef)
    {
        return;
    }

    // Apply OnEquippedEffects
    for (const TSubclassOf<UGameplayEffect>& EffectClass : ItemDef->OnEquippedEffects)
    {
        if (EffectClass)
        {
            FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
            Context.AddSourceObject(GetOwner());
            FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1, Context);
            if (SpecHandle.IsValid())
            {
                FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
                if (EffectHandle.IsValid())
                {
                    EquippedItem.ActiveEffects.Add(EffectHandle);
                }
            }
        }
    }

    // Grant abilities
    for (const TSubclassOf<UGameplayAbility>& AbilityClass : ItemDef->GrantedAbilities)
    {
        if (AbilityClass)
        {
            FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, GetOwner());
            FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AbilitySpec);
            if (Handle.IsValid())
            {
                EquippedItem.GrantedAbilityHandles.Add(Handle);
            }
        }
    }
}

void UREquipmentComponent::RemoveEquipEffects(FREquippedItem& EquippedItem)
{
    UAbilitySystemComponent* ASC = GetOwnerASC();
    if (!ASC)
    {
        return;
    }

    // Remove active effects
    for (const FActiveGameplayEffectHandle& EffectHandle : EquippedItem.ActiveEffects)
    {
        if (EffectHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(EffectHandle);
        }
    }
    EquippedItem.ActiveEffects.Empty();

    // Remove granted abilities
    for (const FGameplayAbilitySpecHandle& AbilityHandle : EquippedItem.GrantedAbilityHandles)
    {
        if (AbilityHandle.IsValid())
        {
            ASC->ClearAbility(AbilityHandle);
        }
    }
    EquippedItem.GrantedAbilityHandles.Empty();
}

UAbilitySystemComponent* UREquipmentComponent::GetOwnerASC() const
{
    if (AActor* Owner = GetOwner())
    {
        if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
        {
            return ASI->GetAbilitySystemComponent();
        }
    }
    return nullptr;
}
