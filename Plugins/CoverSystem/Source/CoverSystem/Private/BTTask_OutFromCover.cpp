


#include "BTTask_OutFromCover.h"

#include "AIController.h"
#include "CoverComponent.h"

UBTTask_OutFromCover::UBTTask_OutFromCover(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Out From Cover";
}

EBTNodeResult::Type UBTTask_OutFromCover::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	return CoverComp->OutFromCover() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
