#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RItemType.h"
#include "RItemDefinition.generated.h"

UCLASS(BlueprintType)
class RIZZGAME_API URItemDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /* ───────── Base / shared data ───────── */
 
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FGameplayTag ItemTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    ERItemDataType ItemType = ERItemDataType::Weapon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FRItemVisualData Visual;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stacking")
    bool bIsStackable = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stacking",
        meta = (EditCondition = "bIsStackable", ClampMin = "1", UIMin = "1"))
    int32 MaxStackSize = 99;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item",
        meta = (ClampMin = "0.0", UIMin = "0.0"))
    float Weight = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TArray<FRAttributeRequirement> AttributeRequirements;

    // GAS: passives and abilities while equipped / used
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|GAS")
    TArray<TSubclassOf<UGameplayEffect>> OnEquippedEffects;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|GAS")
    TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|GAS")
    TArray<TSubclassOf<UGameplayEffect>> OnUseEffects;

    /* ───────── Weapon-specific ───────── */

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Weapon",
        meta = (EditCondition = "ItemType == ERItemDataType::Weapon", EditConditionHides))
    FRDamageRange WeaponDamage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Weapon",
        meta = (EditCondition = "ItemType == ERItemDataType::Weapon", EditConditionHides))
    ERWeaponRangeType RangeType = ERWeaponRangeType::Melee;
 
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Weapon",
        meta = (EditCondition = "ItemType == ERItemDataType::Weapon", EditConditionHides, ClampMin = "0.0"))
    float AttackRange = 5.f;

    /* ───────── Shield-specific ───────── */

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Shield",
        meta = (EditCondition = "ItemType == ERItemDataType::Shield", EditConditionHides))
    FRShieldValues ShieldValues;

    /* ───────── Consumable-specific ───────── */ 

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Consumable|GAS",
        meta = (EditCondition = "ItemType == ERItemDataType::Consumable", EditConditionHides))
    TSubclassOf<UGameplayEffect> ConsumeEffect;

    /* ───────── Misc-specific ───────── */

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Misc",
        meta = (EditCondition = "ItemType == ERItemDataType::Misc", EditConditionHides))
    bool bIsQuestItem = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Misc",
        meta = (EditCondition = "ItemType == ERItemDataType::Misc", EditConditionHides))
    bool bDestroyOnUse = false;

    /* ───────── Helper API ───────── */

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Item")
    bool IsStackable() const
    {
        return bIsStackable && MaxStackSize > 1;
    }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Item")
    float GetWeightPerUnit() const
    {
        return Weight;
    }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Item")
    ERItemSlot GetDefaultSlot() const
    {
        return Visual.DefaultSlot;
    }

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    { 
        return FPrimaryAssetId(TEXT("Item"), GetFName());
    }
};
