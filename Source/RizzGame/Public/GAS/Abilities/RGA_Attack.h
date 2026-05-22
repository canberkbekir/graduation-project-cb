// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/RCharacterBase.h"
#include "Core/RGameplayAbilityBase.h"
#include "Subsystems/DiceSubsystem.h"
#include "RGA_Attack.generated.h"

/**
 * One secondary status effect entry.
 * Add as many as needed in the ability's SecondaryEffects array.
 * ResistanceType and DCSource are configured on the GE asset itself (URStatusGameplayEffect).
 */
USTRUCT(BlueprintType)
struct FSecondaryEffectEntry
{
	GENERATED_BODY()

	/** The status GE to apply. Must be a child of URStatusGameplayEffect. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Secondary Effect")
	TSubclassOf<UGameplayEffect> EffectClass;

	/** Turns applied when the target fails the saving throw. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Secondary Effect", meta=(ClampMin="0"))
	int32 TurnsOnFail = 2;

	/** Turns applied when the target succeeds the saving throw. 0 = no effect on success. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Secondary Effect", meta=(ClampMin="0"))
	int32 TurnsOnSuccess = 0;
};

UCLASS()
class RIZZGAME_API URGA_Attack : public URGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="Ability|Effect")
	FDiceExpression DamageRoll;

	UPROPERTY(EditDefaultsOnly, Category="Ability|Effect")
	FGameplayTag DamageType;

	UPROPERTY(EditDefaultsOnly, Category="Ability|Effect")
	EDiceRollContext DiceRollContext = EDiceRollContext::Combat;

	/**
	 * Optional secondary status effects applied after damage lands.
	 * Each entry rolls its own saving throw independently.
	 * Leave empty for pure damage attacks.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Ability|Secondary Effects")
	TArray<FSecondaryEffectEntry> SecondaryEffects;

protected:
	virtual void CommitAbilityExecution_Implementation() override;

private:
	UFUNCTION()
	void OnMontageNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

	void DoApplyDamageEffect();
};
