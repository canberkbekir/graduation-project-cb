


#include "BTTask_FreeCoverPoint.h"
#include "AIController.h"
#include "CoverComponent.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTTask_FreeCoverPoint::UBTTask_FreeCoverPoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "FreeCoverPoint";
}

EBTNodeResult::Type UBTTask_FreeCoverPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!Controller || !BlackboardComp)
		return EBTNodeResult::Failed;
	
	AActor* ActorOwner = Controller->GetPawn();
	if (!ActorOwner)
		return EBTNodeResult::Failed;
	
	UCoverComponent* CoverComp = ActorOwner->FindComponentByClass<UCoverComponent>();
	if (!CoverComp)		
		return EBTNodeResult::Failed;
	
	CoverComp->ClearCoverPoint();
	return EBTNodeResult::Succeeded;
}
