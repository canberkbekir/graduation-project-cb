// RStatusGameplayEffect.cpp
#include "GAS/Effects/RStatusGameplayEffect.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/Attributes/RCharacterCoreAttributeSet.h"

bool URStatusGameplayEffect::GetSaveInfo(TSubclassOf<UGameplayEffect> EffectClass,
	bool& bOutRequiresSave, EResistanceType& OutResistanceType, int32& OutDC)
{
	if (!EffectClass)
	{
		return false;
	}

	const URStatusGameplayEffect* CDO = Cast<URStatusGameplayEffect>(EffectClass->GetDefaultObject());
	if (!CDO)
	{
		return false;
	}

	bOutRequiresSave  = CDO->bRequiresSavingThrow;
	OutResistanceType = CDO->ResistanceType;
	OutDC             = CDO->AbilityDC;
	return true;
}

int32 URStatusGameplayEffect::ComputeDC(TSubclassOf<UGameplayEffect> EffectClass, AActor* Source)
{
	if (!EffectClass)
	{
		return 10;
	}

	const URStatusGameplayEffect* CDO = Cast<URStatusGameplayEffect>(EffectClass->GetDefaultObject());
	if (!CDO)
	{
		return 10;
	}

	if (CDO->DCSource == EDCSource::Fixed)
	{
		return CDO->AbilityDC;
	}

	if (!Source)
	{
		return 10;
	}

	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Source);
	if (!ASC)
	{
		return 10;
	}

	const URCharacterCoreAttributeSet* Core = ASC->GetSet<URCharacterCoreAttributeSet>();
	if (!Core)
	{
		return 10;
	}

	switch (CDO->DCSource)
	{
	case EDCSource::Physique: return FMath::RoundToInt(Core->GetPhysique());
	case EDCSource::Tech:     return FMath::RoundToInt(Core->GetTech());
	case EDCSource::Mind:     return FMath::RoundToInt(Core->GetMind());
	case EDCSource::Insight:  return FMath::RoundToInt(Core->GetInsight());
	case EDCSource::Finesse:  return FMath::RoundToInt(Core->GetFinesse());
	default:                  return CDO->AbilityDC;
	}
}
