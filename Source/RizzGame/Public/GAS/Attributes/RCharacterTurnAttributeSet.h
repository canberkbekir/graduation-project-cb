// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h" 
#include "RCharacterTurnAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class RIZZGAME_API URCharacterTurnAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData ActionPoints = 2;
	ATTRIBUTE_ACCESSORS(URCharacterTurnAttributeSet, ActionPoints);

	/** Max walk distance per turn, in feet. Effects can modify this. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData MaxWalkDistance = 30.f;
	ATTRIBUTE_ACCESSORS(URCharacterTurnAttributeSet, MaxWalkDistance);

	/** Remaining walk distance for the current turn, in feet. Reset to MaxWalkDistance each turn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData WalkDistance = 30.f;
	ATTRIBUTE_ACCESSORS(URCharacterTurnAttributeSet, WalkDistance);
};
