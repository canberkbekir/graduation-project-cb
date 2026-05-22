// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h" 
class ARCharacterBase;

#include "RCharacterCombatAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class RIZZGAME_API URCharacterCombatAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

	//TODO: Add combat related attributes here like Armor, Evasion, Accuracy, etc.
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData MaxHealth = 100.f;
	ATTRIBUTE_ACCESSORS(URCharacterCombatAttributeSet, MaxHealth);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData Health = 100.f;
	ATTRIBUTE_ACCESSORS(URCharacterCombatAttributeSet, Health);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData KineticShields = 0.f;
	ATTRIBUTE_ACCESSORS(URCharacterCombatAttributeSet, KineticShields);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData MaxKineticShields = 0.f;
	ATTRIBUTE_ACCESSORS(URCharacterCombatAttributeSet, MaxKineticShields);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData EnergyShields= 0.f;
	ATTRIBUTE_ACCESSORS(URCharacterCombatAttributeSet, EnergyShields);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData MaxEnergyShields = 0.f;
	ATTRIBUTE_ACCESSORS(URCharacterCombatAttributeSet, MaxEnergyShields);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trauma")
	FGameplayAttributeData FreshTrauma;
	ATTRIBUTE_ACCESSORS(URCharacterCombatAttributeSet, FreshTrauma);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trauma")
	FGameplayAttributeData OldTrauma;
	ATTRIBUTE_ACCESSORS(URCharacterCombatAttributeSet, OldTrauma);

private:
	virtual void PreAttributeChange(
		const FGameplayAttribute& Attribute,
		float& NewValue) override;

	virtual void PostGameplayEffectExecute(
		const FGameplayEffectModCallbackData& Data) override;

	
};
