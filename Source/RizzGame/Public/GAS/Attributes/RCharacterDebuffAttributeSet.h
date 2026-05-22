// RCharacterDebuffAttributeSet.h
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "RCharacterDebuffAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Holds runtime modifiers applied by status conditions.
 * GE Modifiers write to these attributes (Additive); GAS removes them automatically when the GE expires.
 *
 * Designer workflow: open any status GE asset → Modifiers section → pick an attribute here →
 * set Additive magnitude (e.g. -2 for a -2 penalty). No code changes needed per new condition.
 */
UCLASS()
class RIZZGAME_API URCharacterDebuffAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	/** Added to the attacker's 1d20 roll in PerformHitCheck. Negative = hit penalty (Disrupted Senses). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debuffs")
	FGameplayAttributeData AttackRollModifier = 0.f;
	ATTRIBUTE_ACCESSORS(URCharacterDebuffAttributeSet, AttackRollModifier);

	/** Added to the defender's Dodge value in GetDodgeValue. Negative = easier to hit (Destabilized). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debuffs")
	FGameplayAttributeData DefenseRollModifier = 0.f;
	ATTRIBUTE_ACCESSORS(URCharacterDebuffAttributeSet, DefenseRollModifier);

	/** Added to the target's stat modifier on ALL saving throws in PerformSavingThrow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debuffs")
	FGameplayAttributeData SaveRollModifier = 0.f;
	ATTRIBUTE_ACCESSORS(URCharacterDebuffAttributeSet, SaveRollModifier);

	/** Added to the target's stat modifier on Tech/Network saving throws only (Disrupted Senses -2). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debuffs")
	FGameplayAttributeData TechSaveModifier = 0.f;
	ATTRIBUTE_ACCESSORS(URCharacterDebuffAttributeSet, TechSaveModifier);

	/** Fraction of extra Network damage received. 0.15 = +15% Network damage (Corrupted condition). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debuffs")
	FGameplayAttributeData NetworkDamageVulnerability = 0.f;
	ATTRIBUTE_ACCESSORS(URCharacterDebuffAttributeSet, NetworkDamageVulnerability);

	/** Fraction of extra Physical damage received. Reserved for future conditions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debuffs")
	FGameplayAttributeData PhysicalDamageVulnerability = 0.f;
	ATTRIBUTE_ACCESSORS(URCharacterDebuffAttributeSet, PhysicalDamageVulnerability);

	/** Flat bonus added to incoming Network damage after firewall resistance (Corrupted condition).
	 *  GE Modifier writes caster's TechMod here via AttributeBased Additive. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debuffs")
	FGameplayAttributeData NetworkDamageFlatBonus = 0.f;
	ATTRIBUTE_ACCESSORS(URCharacterDebuffAttributeSet, NetworkDamageFlatBonus);

	/** Flat bonus added to incoming Physical damage after resistance (future conditions). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debuffs")
	FGameplayAttributeData PhysicalDamageFlatBonus = 0.f;
	ATTRIBUTE_ACCESSORS(URCharacterDebuffAttributeSet, PhysicalDamageFlatBonus);
};
