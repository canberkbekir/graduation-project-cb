

#include "BTTask_OccupyCoverPoint.h"

#include "AIController.h"
#include "CoverComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_OccupyCoverPoint::UBTTask_OccupyCoverPoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Occupy Cover Point";
	CoverPointKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_OccupyCoverPoint, CoverPointKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_OccupyCoverPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	
	/*if (!CoverPoint->IsAvailable() && !CoverPoint->IsReservedBy(ActorOwner))
	{
		BlackboardComp->ClearValue(CoverPointKey.SelectedKeyName);
		return EBTNodeResult::Failed;
	}
	
	if (bIsReservedBeforeOccupying)
		CoverComp->OccupyCoverPoint(CoverPoint);
	else
		CoverComp->OccupyCoverPointWithoutReservation(CoverPoint);*/
	
	if (bIsReservedBeforeOccupying && CoverPoint->IsReservedBy(ActorOwner))
	{
		CoverComp->OccupyCoverPoint(CoverPoint, bIsPhysicllyReached);
		return EBTNodeResult::Succeeded;
	}
	if (!bIsReservedBeforeOccupying && CoverPoint->IsAvailable())
	{
		CoverComp->OccupyCoverPointWithoutReservation(CoverPoint, bIsPhysicllyReached);
		return EBTNodeResult::Succeeded;
	}
	
	return EBTNodeResult::Failed;
}
