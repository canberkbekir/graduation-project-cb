// AbilityEvents.h
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "Subsystems/DiceSubsystem.h"
#include "AbilityEvents.generated.h"

/**
 * Broadcast by URGameplayAbilityBase::RollAndBroadcast / ApplyEffectWithRoll
 * whenever any ability resolves a dice-based outcome.
 *
 * Subscribe here (not to abilities directly) to react to ability resolutions:
 * log entries, UI floating text, AI threat scoring, achievements, etc.
 */
USTRUCT(BlueprintType)
struct FAbilityResolved
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Source;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Target;

	/** SetByCaller tag used for the GE magnitude (e.g. Data.Damage.Physical). */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag MagnitudeTag;

	UPROPERTY(BlueprintReadOnly)
	FDiceExpression Expression;

	UPROPERTY(BlueprintReadOnly)
	FDiceRollResult Result;

	UPROPERTY(BlueprintReadOnly)
	EDiceRollContext RollContext = EDiceRollContext::None;

	/** Class of the ability that produced this resolution. */
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> AbilityClass;
};
