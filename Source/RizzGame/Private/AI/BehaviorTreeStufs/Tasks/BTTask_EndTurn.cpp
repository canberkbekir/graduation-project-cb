


#include "AI/BehaviorTreeStufs/Tasks/BTTask_EndTurn.h"

#include "AIController.h"
#include "Components/RTurnComponent.h"
#include "Core/RCharacterBase.h"

UBTTask_EndTurn::UBTTask_EndTurn(const FObjectInitializer& InitializerModule) : Super(InitializerModule)
{
	NodeName = "End Turn";
}

EBTNodeResult::Type UBTTask_EndTurn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (!AIController)		
		return EBTNodeResult::Failed;
	ARCharacterBase* EnemyCharacter = Cast<ARCharacterBase>(AIController->GetPawn());
	if (!EnemyCharacter)
		return EBTNodeResult::Failed;
	URTurnComponent* TurnComponent = EnemyCharacter->FindComponentByClass<URTurnComponent>();
	if (!TurnComponent)
		return EBTNodeResult::Failed;
	TurnComponent->EndTurn();
	if (bClearFocus)
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	return EBTNodeResult::Succeeded;
}
