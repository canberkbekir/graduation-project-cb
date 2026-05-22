// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h" 
#include "AbilitySystemComponent.h"
#include "AttributeSet.h" 
#include "RCharacterCoreAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class RIZZGAME_API URCharacterCoreAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData Tech = 8;
	ATTRIBUTE_ACCESSORS(URCharacterCoreAttributeSet, Tech);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData Physique = 8;
	ATTRIBUTE_ACCESSORS(URCharacterCoreAttributeSet, Physique);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData Finesse = 8;
	ATTRIBUTE_ACCESSORS(URCharacterCoreAttributeSet, Finesse);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData Mind = 8;
	ATTRIBUTE_ACCESSORS(URCharacterCoreAttributeSet, Mind);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData Insight = 8; 
	ATTRIBUTE_ACCESSORS(URCharacterCoreAttributeSet, Insight);
};
