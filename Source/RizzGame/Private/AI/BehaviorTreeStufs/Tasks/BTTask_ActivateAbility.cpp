


#include "AI/BehaviorTreeStufs/Tasks/BTTask_ActivateAbility.h"

#include "AbilitySystemComponent.h"
#include "HAIController.h"
#include "AI/EnemyCharacterBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTTask_ActivateAbility::UBTTask_ActivateAbility(const FObjectInitializer& InitializerModule) : Super(InitializerModule)
{
	bCreateNodeInstance = true;
	NodeName = "Activate Ability";
	
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ActivateAbility, TargetActorKey), ARCharacterBase::StaticClass());
	TargetActorKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ActivateAbility, TargetActorKey));
}

EBTNodeResult::Type UBTTask_ActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AHAIController* AIController = Cast<AHAIController>(OwnerComp.GetAIOwner());
	if (!AIController)		
		return EBTNodeResult::Failed;
	AEnemyCharacterBase* EnemyCharacter = Cast<AEnemyCharacterBase>(AIController->GetPawn());
	if (!EnemyCharacter)
		return EBTNodeResult::Failed;
	UAbilitySystemComponent* ASC = EnemyCharacter->GetAbilitySystemComponent();
	if (!ASC)
		return EBTNodeResult::Failed;
	if (!AbilityClass)
	{
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			UClass* SelectedClass = BlackboardComp->GetValueAsClass("ScramblerAbility");
			if (SelectedClass && SelectedClass->IsChildOf(UGameplayAbility::StaticClass()))
				AbilityClass = SelectedClass;
		}
	}
	
	ActivatedAbilityClass = AbilityClass;
	if (bUseRandomAbility && SecondaryAbilityClass)
		if (FMath::RandBool())
			ActivatedAbilityClass = SecondaryAbilityClass;
	if (!ActivatedAbilityClass)
		return EBTNodeResult::Failed;
	
	CachedOwnerComp = &OwnerComp;
	
	EndedHandle = ASC->OnAbilityEnded.AddUObject(this, &UBTTask_ActivateAbility::OnAbilityEndedCallback);
	
	FGameplayAbilitySpec* ExistingSpec = ASC->FindAbilitySpecFromClass(ActivatedAbilityClass);
	FGameplayAbilitySpecHandle SpecHandle = ExistingSpec ? ExistingSpec->Handle : ASC->GiveAbility(FGameplayAbilitySpec(ActivatedAbilityClass, 1));
	ActivatedAbilityHandle = SpecHandle;
	
	FGameplayEventData EventData;
	EventData.Instigator = EnemyCharacter;
    
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		if (Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName)))
		{
			EventData.Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
		}
		else
		{
			FVector TargetLocation = BlackboardComp->GetValueAsVector(TargetActorKey.SelectedKeyName);
			
			FGameplayAbilityTargetData_LocationInfo* LocData = new FGameplayAbilityTargetData_LocationInfo();
			LocData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
			LocData->TargetLocation.LiteralTransform = FTransform(TargetLocation);
            
			EventData.TargetData.Add(LocData);
		}
		EventData.EventMagnitude = AcceptableRadius;
	}
	
	const bool bActivated = ASC->TriggerAbilityFromGameplayEvent(SpecHandle, ASC->AbilityActorInfo.Get(), FGameplayTag(), &EventData, *ASC);

	if (bActivated)
	{
		if (bRecordAsLastAbility)
			EnemyCharacter->LastPlayedAbilityClass = ActivatedAbilityClass;
		return EBTNodeResult::InProgress;
	}
	ASC->OnAbilityEnded.Remove(EndedHandle);
	return EBTNodeResult::Failed;
}

void UBTTask_ActivateAbility::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (AHAIController* AIController = Cast<AHAIController>(OwnerComp.GetAIOwner()))
		if (AEnemyCharacterBase* EnemyCharacter = Cast<AEnemyCharacterBase>(AIController->GetPawn()))
			if (UAbilitySystemComponent* ASC = EnemyCharacter->GetAbilitySystemComponent())
				ASC->OnAbilityEnded.Remove(EndedHandle);
	
	CachedOwnerComp = nullptr;
	ActivatedAbilityClass = nullptr;
	
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

EBTNodeResult::Type UBTTask_ActivateAbility::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AHAIController* AIController = Cast<AHAIController>(OwnerComp.GetAIOwner()))
		if (AEnemyCharacterBase* EnemyCharacter = Cast<AEnemyCharacterBase>(AIController->GetPawn()))
			if (UAbilitySystemComponent* ASC = EnemyCharacter->GetAbilitySystemComponent())
			{
				ASC->OnAbilityEnded.Remove(EndedHandle);
				ASC->CancelAbilityHandle(ActivatedAbilityHandle);
			}
    
	CachedOwnerComp = nullptr;
	return EBTNodeResult::Aborted;
}

void UBTTask_ActivateAbility::OnAbilityEndedCallback(const FAbilityEndedData& EndedData)
{
	if (EndedData.AbilityThatEnded->GetClass() == ActivatedAbilityClass)
		if (CachedOwnerComp)
			FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}
