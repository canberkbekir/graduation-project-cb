


#include "AI/BehaviorTreeStufs/Decorators/BTDecorator_CheckTargetHealth.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/RCharacterBase.h"
#include "GAS/Attributes/RCharacterCombatAttributeSet.h"

UBTDecorator_CheckTargetHealth::UBTDecorator_CheckTargetHealth(const FObjectInitializer& InitializerModule) : Super(InitializerModule)
{
	NodeName = "CheckTargetHealth";
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckTargetHealth, TargetActorKey), ARCharacterBase::StaticClass());
}

bool UBTDecorator_CheckTargetHealth::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return false;
	ARCharacterBase* TargetCharacter = Cast<ARCharacterBase>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetCharacter)
		return false;
	auto combatAt = TargetCharacter->GetCombatAttributes();
	if (!combatAt)
		return false;
	float CurrentHealthPercentage = combatAt->GetHealth() / combatAt->GetMaxHealth();
	if (bBelow)
		return CurrentHealthPercentage < HealthPertange;
	return CurrentHealthPercentage >= HealthPertange;
}
