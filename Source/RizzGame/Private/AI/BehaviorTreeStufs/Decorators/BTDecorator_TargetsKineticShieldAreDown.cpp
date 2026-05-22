


#include "AI/BehaviorTreeStufs/Decorators/BTDecorator_TargetsKineticShieldAreDown.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Core/RCharacterBase.h"
#include "GAS/Attributes/RCharacterCombatAttributeSet.h"

UBTDecorator_TargetsKineticShieldAreDown::UBTDecorator_TargetsKineticShieldAreDown(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "TargetsKineticShieldAreDown";
}

bool UBTDecorator_TargetsKineticShieldAreDown::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) const
{
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (!AIController) 		
		return false;
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
		return false;
	ARCharacterBase* TargetActor = Cast<ARCharacterBase>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)
		return false;
	return TargetActor->GetCombatAttributes()->KineticShields.GetCurrentValue() <= 0.f;
}
