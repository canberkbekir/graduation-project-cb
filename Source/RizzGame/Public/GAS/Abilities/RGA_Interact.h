#pragma once

#include "CoreMinimal.h"
#include "AITypes.h"
#include "Core/RGameplayAbilityBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "RGA_Interact.generated.h"

class AAIController;
class APawn;
class URInteractionAreaComponent;

UCLASS()
class RIZZGAME_API URGA_Interact : public URGameplayAbilityBase
{
	GENERATED_BODY()

public:
	URGA_Interact();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual bool CheckCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual bool CheckCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ApplyCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rizz|Interaction")
	float DefaultInteractionRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rizz|Interaction")
	bool bRotateToFaceInteractionPoint;

	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);
	void TryNextArea(APawn* Pawn);
	bool IssueMoveToLocation(AAIController* AICon, APawn* Pawn, const FVector& Location, float AcceptanceRadius);
	void PerformInteraction(APawn* Pawn);

	virtual float GetEffectiveRange() const override;

private:
	TWeakObjectPtr<AActor> CurrentInteractable;
	TArray<TWeakObjectPtr<URInteractionAreaComponent>> PendingAreas;
	int32 PendingAreaIndex;
	FAIRequestID ActiveMoveRequestID;
	bool bHasActiveMoveRequest;

	// Defers PerformInteraction one tick to keep EndAbility outside the GAS scope lock.
	FTimerHandle InRangeInteractionTimer;
	void OnInRangeTimerFired();
};
