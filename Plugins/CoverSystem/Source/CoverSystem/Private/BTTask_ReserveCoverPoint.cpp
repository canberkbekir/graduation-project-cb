


#include "BTTask_ReserveCoverPoint.h"

#include "AIController.h"
#include "CoverComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ReserveCoverPoint::UBTTask_ReserveCoverPoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Reserve Cover Point";
	CoverPointKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ReserveCoverPoint, CoverPointKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_ReserveCoverPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	
	ACoverPoint* CoverPoint = Cast<ACoverPoint>(BlackboardComp->GetValueAsObject(CoverPointKey.SelectedKeyName));
	if (!CoverPoint)		
		return EBTNodeResult::Failed;
	
	if (!CoverPoint->IsAvailable())
		return EBTNodeResult::Failed;
	
	CoverComp->ReserveCoverPoint(CoverPoint);
	
	return EBTNodeResult::Succeeded;
}
