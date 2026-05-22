


#include "BTTask_BackToCover.h"
#include "AIController.h"
#include "CoverComponent.h"

UBTTask_BackToCover::UBTTask_BackToCover(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Back To Cover";
}

EBTNodeResult::Type UBTTask_BackToCover::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	return CoverComp->BackToCover() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
