
#include "Public/AI/ActivateAbilityStateTreeTask.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "StateTreeExecutionContext.h"


EStateTreeRunStatus FRTask_ActivateAbility::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstanceData.Actor);

	if (!ASC || !AbilityClass)
		return EStateTreeRunStatus::Failed;
	
	InstanceData.EndedHandle = ASC->OnAbilityEnded.AddWeakLambda(InstanceData.Actor.Get(), [&InstanceData, this](const FAbilityEndedData& EndedData)
	{
		if (EndedData.AbilityThatEnded->GetClass() == AbilityClass)
			InstanceData.bAbilityEnded = true;
	});
	
	FGameplayAbilitySpec* ExistingSpec = ASC->FindAbilitySpecFromClass(AbilityClass);
	FGameplayAbilitySpecHandle SpecHandle = ExistingSpec ? ExistingSpec->Handle : ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));

	FGameplayEventData EventData;
	EventData.Instigator = InstanceData.Actor;
	EventData.Target = InstanceData.TargetActor;

	const bool bActivated = ASC->TriggerAbilityFromGameplayEvent(SpecHandle, ASC->AbilityActorInfo.Get(), FGameplayTag(), &EventData, *ASC);

	UE_LOG(LogTemp, Warning, TEXT("TriggerAbilityFromGameplayEvent returned: %s"), bActivated ? TEXT("true") : TEXT("false"));

    return bActivated ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FRTask_ActivateAbility::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
	if (InstanceData.bAbilityEnded)
		return EStateTreeRunStatus::Succeeded;
    
	return EStateTreeRunStatus::Running;
}

void FRTask_ActivateAbility::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstanceData.Actor);
	
	if (ASC)
		ASC->OnAbilityEnded.Remove(InstanceData.EndedHandle);
}
