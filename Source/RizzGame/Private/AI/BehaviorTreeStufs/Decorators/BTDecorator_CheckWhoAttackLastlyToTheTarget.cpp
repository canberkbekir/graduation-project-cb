


#include "AI/BehaviorTreeStufs/Decorators/BTDecorator_CheckWhoAttackLastlyToTheTarget.h"

UBTDecorator_CheckWhoAttackLastlyToTheTarget::UBTDecorator_CheckWhoAttackLastlyToTheTarget(const FObjectInitializer& InitializerModule) : Super(InitializerModule)
{
	NodeName = "CheckWhoAttackLastlyToTheTarget";
}

bool UBTDecorator_CheckWhoAttackLastlyToTheTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	return true;
}
