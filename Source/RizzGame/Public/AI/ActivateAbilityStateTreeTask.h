#pragma once
#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "Abilities/GameplayAbility.h"
#include "ActivateAbilityStateTreeTask.generated.h"

USTRUCT()
struct FRTask_ActivateAbilityInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category= "Context")
	TObjectPtr<AActor> Actor = nullptr;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;
	
	bool bAbilityEnded = false;
	
	FDelegateHandle EndedHandle;
};

USTRUCT(DisplayName = "Activate Ability", Category = "Combat")
struct FRTask_ActivateAbility : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRTask_ActivateAbilityInstanceData;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<UGameplayAbility> AbilityClass;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};