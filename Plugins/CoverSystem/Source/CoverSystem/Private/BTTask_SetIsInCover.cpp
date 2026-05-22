


#include "BTTask_SetIsInCover.h"

#include "AIController.h"
#include "CoverComponent.h"

UBTTask_SetIsInCover::UBTTask_SetIsInCover(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Set Is In Cover";
}

EBTNodeResult::Type UBTTask_SetIsInCover::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return EBTNodeResult::Failed;
	AActor* owner = AIController->GetPawn();
	if (!owner)
		return EBTNodeResult::Failed;
	UCoverComponent* CoverComp = owner->FindComponentByClass<UCoverComponent>();
	if (!CoverComp)
		return EBTNodeResult::Failed;
	CoverComp->SetIsInCover(bIsInCover);
	return EBTNodeResult::Succeeded;
}
