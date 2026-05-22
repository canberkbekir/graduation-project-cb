


#include "AI/BehaviorTreeStufs/Decorators/BTDecorator_ShieldsAreDown.h"
#include "AIController.h"
#include "Core/RCharacterBase.h"
#include "GAS/Attributes/RCharacterCombatAttributeSet.h"

UBTDecorator_ShieldsAreDown::UBTDecorator_ShieldsAreDown(const FObjectInitializer& InitializerModule) : Super(InitializerModule)
{
	NodeName = "ShieldsAreDown";
}

bool UBTDecorator_ShieldsAreDown::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return false;
	ARCharacterBase* EnemyCharacter = Cast<ARCharacterBase>(AIController->GetPawn());
	if (!EnemyCharacter)
		return false;
	auto combatAttributeComponent = EnemyCharacter->GetCombatAttributes();
	if (!combatAttributeComponent)
		return false;
	if (bIsKineticShield)
		return combatAttributeComponent->KineticShields.GetCurrentValue() <= 0.f;
	return combatAttributeComponent->EnergyShields.GetCurrentValue() <= 0.f;
}
