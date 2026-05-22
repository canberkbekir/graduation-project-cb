// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Attributes/RCharacterCombatAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Core/RCharacterBase.h"


void URCharacterCombatAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}

}

void URCharacterCombatAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));

		if (GetHealth() <= 0.f)
		{
			if (ARCharacterBase* OwnerChar = Cast<ARCharacterBase>(GetOwningActor()))
			{
				if (!OwnerChar->IsDead())
				{
					// Resolve killer from the effect causer — null for environmental/DOT deaths.
					ARCharacterBase* Killer = Cast<ARCharacterBase>(
						Data.EffectSpec.GetEffectContext().GetEffectCauser());
					OwnerChar->HandleDeath(Killer);
				}
			}
		}
		else if (GetHealth() < GetMaxHealth() * 0.25f)
		{
			// Only accumulate trauma on living characters.
			SetFreshTrauma(GetFreshTrauma() + 1);
		}
	}

	if (Data.EvaluatedData.Attribute == GetFreshTraumaAttribute())
	{
		if (GetFreshTrauma() >= 10)
		{
			SetOldTrauma(GetOldTrauma() + 1);
			SetFreshTrauma(0);
		}
	}
}