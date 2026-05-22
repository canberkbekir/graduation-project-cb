// CheckEvents.h
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "Subsystems/DiceSubsystem.h"
#include "CheckEvents.generated.h"

/**
 * Broadcast by URGameplayAbilityBase::PerformHitCheck and PerformSavingThrow
 * for every combat check roll (hit checks, saving throws).
 *
 * Subscribe here to drive log entries, UI floating text, and combat feedback
 * for rolls that do not directly apply a GameplayEffect magnitude.
 */
USTRUCT(BlueprintType)
struct FCheckResolved
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Source;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Target;

	/** Identifies the check type. Registered tags: Check.Hit, Check.Save.Physical,
	 *  Check.Save.Network, Check.Save.Mental, Check.Save.Radiation, Check.Save.Electromagnetic. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag CheckTag;

	UPROPERTY(BlueprintReadOnly)
	FDiceExpression Expression;

	UPROPERTY(BlueprintReadOnly)
	FDiceRollResult Roll;

	/** Target number the roll was compared against (Dodge value or AbilityDC). */
	UPROPERTY(BlueprintReadOnly)
	int32 TargetValue = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	/** True when the roll was a natural 20 (critical hit or critical success save). */
	UPROPERTY(BlueprintReadOnly)
	bool bCritical = false;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> AbilityClass;
};
