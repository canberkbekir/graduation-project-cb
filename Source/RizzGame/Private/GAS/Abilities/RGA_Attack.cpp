// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/RGA_Attack.h"

#include "AbilitySystemComponent.h"
#include "Core/RCharacterBase.h"
#include "GAS/Attributes/RCharacterTurnAttributeSet.h"

void URGA_Attack::CommitAbilityExecution_Implementation()
{
	const UAbilitySystemComponent* Asc = CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr;
	const URCharacterTurnAttributeSet* Attrs = Asc ? Asc->GetSet<URCharacterTurnAttributeSet>() : nullptr;
	const float APBefore = Attrs ? Attrs->GetActionPoints() : -1.f;

	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		UE_LOG(LogHAL, Warning, TEXT("[AP] Attack CommitAbility FAILED (AP=%.0f)"), APBefore);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UE_LOG(LogHAL, Log, TEXT("[AP] Attack — cost: %.0f | remaining: %.0f"),
		APBefore - (Attrs ? Attrs->GetActionPoints() : APBefore), Attrs ? Attrs->GetActionPoints() : -1.f);

	ARCharacterBase* Character = Cast<ARCharacterBase>(CurrentActorInfo->AvatarActor.Get());
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (!AbilityExecuteMontage)
	{
		// No execute montage set — apply effect immediately without animation.
		DoApplyDamageEffect();
		return;
	}

	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	AnimInstance->Montage_Play(AbilityExecuteMontage);
	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &URGA_Attack::OnMontageNotify);

	UE_LOG(LogHAL, Log, TEXT("%s: Attack montage started"), *CurrentActorInfo->AvatarActor->GetName());
}

void URGA_Attack::OnMontageNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	if (const ARCharacterBase* Character = Cast<ARCharacterBase>(Payload.SkelMeshComponent->GetOwner()))
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &URGA_Attack::OnMontageNotify);
		}
	}
	DoApplyDamageEffect();
}

void URGA_Attack::DoApplyDamageEffect()
{
	ARCharacterBase* Target = Cast<ARCharacterBase>(HitResult.GetActor());
	if (!Target)
	{
		UE_LOG(LogHAL, Warning, TEXT("URGA_Attack: No valid target in HitResult"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	if (!GameplayEffectClass)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
	if (!TargetASC)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	AActor* Source = CurrentActorInfo ? CurrentActorInfo->AvatarActor.Get() : nullptr;

	// Hit check: roll 1d20 + AttackMod vs target's Dodge.
	FHitCheckResult CombatHit = PerformHitCheck(Source, Target, AttackMod, DiceRollContext);
	if (CombatHit.Outcome == EHitOutcome::Miss)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	const float DamageMult = (CombatHit.Outcome == EHitOutcome::CriticalHit) ? 2.f : 1.f;
	ApplyEffectWithRoll(Source, Target, GameplayEffectClass, DamageRoll, DamageType, DiceRollContext, DamageMult);

	UE_LOG(LogHAL, Log, TEXT("URGA_Attack: Damage applied to %s%s"),
		*Target->GetName(),
		(CombatHit.Outcome == EHitOutcome::CriticalHit) ? TEXT(" [CRITICAL HIT]") : TEXT(""));

	UE_LOG(LogHAL, Log, TEXT("URGA_Attack: %d secondary effect(s) to process"), SecondaryEffects.Num());
	for (const FSecondaryEffectEntry& SE : SecondaryEffects)
	{
		if (!SE.EffectClass)
		{
			UE_LOG(LogHAL, Warning, TEXT("URGA_Attack: SecondaryEffect entry has null EffectClass — set it in the ability defaults"));
			continue;
		}

		UE_LOG(LogHAL, Log, TEXT("URGA_Attack: Rolling save for %s"), *SE.EffectClass->GetName());
		const FSaveCheckResult Save = PerformSavingThrowForEffect(Source, Target, SE.EffectClass, DiceRollContext);
		if (Save.Outcome == ESaveOutcome::CriticalSuccess)
		{
			UE_LOG(LogHAL, Log, TEXT("URGA_Attack: Critical save — effect negated"));
			continue;
		}

		const int32 Turns = (Save.Outcome == ESaveOutcome::Success) ? SE.TurnsOnSuccess : SE.TurnsOnFail;
		UE_LOG(LogHAL, Log, TEXT("URGA_Attack: Save %s → applying %s for %d turn(s)"),
			(Save.Outcome == ESaveOutcome::Success) ? TEXT("SUCCESS") : TEXT("FAIL"),
			*SE.EffectClass->GetName(), Turns);

		if (Turns > 0)
		{
			ApplyEffectWithRoll(Source, Target, SE.EffectClass, FDiceExpression(), FGameplayTag(), DiceRollContext, 1.f, Turns);
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
