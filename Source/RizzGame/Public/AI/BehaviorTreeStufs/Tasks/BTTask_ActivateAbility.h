

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Abilities/GameplayAbility.h"
#include "BTTask_ActivateAbility.generated.h"

UCLASS()
class RIZZGAME_API UBTTask_ActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_ActivateAbility(const FObjectInitializer& InitializerModule);
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<UGameplayAbility> AbilityClass;
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bUseRandomAbility = false;
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (EditCondition = "bUseRandomAbility"))
	TSubclassOf<UGameplayAbility> SecondaryAbilityClass;
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	bool bRecordAsLastAbility = true;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptableRadius = 5.0f;
private:
	void OnAbilityEndedCallback(const FAbilityEndedData& EndedData);
	
	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp;
	
	FDelegateHandle EndedHandle;
	
	UPROPERTY()
	TSubclassOf<UGameplayAbility> ActivatedAbilityClass;
	UPROPERTY()
	FGameplayAbilitySpecHandle ActivatedAbilityHandle;
};
