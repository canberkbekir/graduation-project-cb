

#include "BTTask_SetCoverPosition.h"

#include "AIController.h"
#include "CoverComponent.h"


UBTTask_SetCoverPosition::UBTTask_SetCoverPosition(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Set Cover Position";
}

EBTNodeResult::Type UBTTask_SetCoverPosition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!Controller || !BlackboardComp)
		return EBTNodeResult::Failed;
	
	AActor* Owner = Controller->GetPawn();
	if (!Owner)
		return EBTNodeResult::Failed;
	UCoverComponent* CoverComp = Owner->FindComponentByClass<UCoverComponent>();
	if (!CoverComp)
		return EBTNodeResult::Failed;
	
	ACoverPoint* CoverPoint = CoverComp->GetCoverPoint();
	if (!CoverPoint)
		return EBTNodeResult::Failed;
	auto coverState = CoverPoint->CoverType;
	switch (coverState)
	{
		case ECoverType::CrouchCover:
			CoverComp->SetCrouchCoverPosition(CrouchPosition);
			break;
		case ECoverType::StandCover:
			CoverComp->SetStandCoverPosition(StandPosition);
			break;
	}
	return EBTNodeResult::Succeeded;
}
