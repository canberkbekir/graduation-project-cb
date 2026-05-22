// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Core/RGameplayAbilityBase.h"
#include "Core/RCharacterBase.h"
#include "RGA_Walk.generated.h"

/**
 *
 */
UCLASS()
class RIZZGAME_API URGA_Walk : public URGameplayAbilityBase
{
	GENERATED_BODY()

public:
	URGA_Walk();
	/*virtual bool CanActivateAbility(FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;*/

	FVector DesiredTargetLocation;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr)
	const override;



	bool Walk(const ACharacter* Character);
	bool TurnWalk(ARCharacterBase* Character);
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
protected:
	virtual float GetEffectiveRange() const override;

private:
	FVector StartLocation;

	void OnMoveFinished(FAIRequestID RequestID, const struct FPathFollowingResult& Result);
	FDelegateHandle MoveDelegateHandle;
	FAIRequestID CachedMoveID;
	UPROPERTY()
	class AAIController* CachedAIController;
};
