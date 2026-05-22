
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RItemType.generated.h"

class UTexture2D;
class UGameplayEffect;
class UGameplayAbility;

/* ───────── Enums ───────── */

UENUM(BlueprintType)
enum class ERItemRarity : uint8
{
    Common      UMETA(DisplayName = "Common"),
    Uncommon    UMETA(DisplayName = "Uncommon"),
    Rare        UMETA(DisplayName = "Rare"),
    Epic        UMETA(DisplayName = "Epic"),
    Legendary   UMETA(DisplayName = "Legendary"),
    Unique      UMETA(DisplayName = "Unique")
};

UENUM(BlueprintType)
enum class ERItemSlot : uint8
{
    None            UMETA(DisplayName = "None"),

    WeaponMainHand  UMETA(DisplayName = "Main Hand"),
    WeaponOffHand   UMETA(DisplayName = "Off Hand"),
    Shield          UMETA(DisplayName = "Shield"),
    WeaponTwoHand   UMETA(DisplayName = "Two Hand"),
};

/** Equipment slot types for character equipment system. */
UENUM(BlueprintType)
enum class EREquipmentSlot : uint8
{
    None        UMETA(DisplayName = "None"),
    Head        UMETA(DisplayName = "Head"),
    Chest       UMETA(DisplayName = "Chest"),
    Hands       UMETA(DisplayName = "Hands"),
    Feet        UMETA(DisplayName = "Feet"),
    MainHand    UMETA(DisplayName = "Main Hand"),
    OffHand     UMETA(DisplayName = "Off Hand"),
    Accessory1  UMETA(DisplayName = "Accessory 1"),
    Accessory2  UMETA(DisplayName = "Accessory 2")
};

UENUM(BlueprintType)
enum class ERItemDamageType : uint8
{
    Physical   UMETA(DisplayName = "Physical"), // Blocked by Kinetic Shield
    Network    UMETA(DisplayName = "Network"),  // Blocked by Energy Shield
};

/* ───────── Small value types ───────── */

USTRUCT(BlueprintType)
struct FRDamageRange
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
    float MinDamage = 1.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
    float MaxDamage = 2.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
    ERItemDamageType DamageType = ERItemDamageType::Physical;
};

/** Base Shield and Resistance values granted by a Shield item. */
USTRUCT(BlueprintType)
struct FRShieldValues
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shield")
    float KineticShield = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shield")
    float EnergyShield = 0.f;

    /** Example: Physical +10%, Network +20% (percent or flat, up to your GEs). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shield")
    TMap<ERItemDamageType, float> Resistances;
};

USTRUCT(BlueprintType)
struct FRAttributeRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Requirement")
    FGameplayTag AttributeTag;     

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Requirement")
    int32 RequiredValue = 0;
};

/** Visual & generic metadata shared by item definitions. */
USTRUCT(BlueprintType)
struct FRItemVisualData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Visual")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Visual")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Visual")
    TSoftObjectPtr<UTexture2D> Icon; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Visual")
    ERItemRarity DefaultRarity = ERItemRarity::Common;

    // Optional: intended equipment slot for this item (used by UI / equip logic).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Visual")
    ERItemSlot DefaultSlot = ERItemSlot::None;
};

UENUM(BlueprintType)
enum class ERWeaponRangeType : uint8
{
    Melee  UMETA(DisplayName = "Melee"),
    Ranged UMETA(DisplayName = "Ranged")
};

// What "kind" of item the data asset is.
UENUM(BlueprintType)
enum class ERItemDataType : uint8
{
    Weapon      UMETA(DisplayName = "Weapon"),
    Shield      UMETA(DisplayName = "Shield"),
    Consumable  UMETA(DisplayName = "Consumable"),
    Misc        UMETA(DisplayName = "Misc / Other")
};