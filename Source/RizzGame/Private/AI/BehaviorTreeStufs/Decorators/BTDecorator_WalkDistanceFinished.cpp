


#include "AI/BehaviorTreeStufs/Decorators/BTDecorator_WalkDistanceFinished.h"
#include "AIController.h"
#include "Core/RCharacterBase.h"
#include "GAS/Attributes/RCharacterTurnAttributeSet.h"

UBTDecorator_WalkDistanceFinished::UBTDecorator_WalkDistanceFinished(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Walk Distance Finished";
}

bool UBTDecorator_WalkDistanceFinished::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (!AIController) 		
		return false;
	ARCharacterBase* EnemyCharacter = Cast<ARCharacterBase>(AIController->GetPawn());
	if (!EnemyCharacter) 	
		return false;
	auto turnAt = EnemyCharacter->GetTurnAttributes();
	if (!turnAt)
		return false;
	return turnAt->GetWalkDistance() <= 0.f;
}
