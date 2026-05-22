

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_TargetsKineticShieldAreDown.generated.h"

UCLASS()
class RIZZGAME_API UBTDecorator_TargetsKineticShieldAreDown : public UBTDecorator
{
	GENERATED_BODY()
public:
	UBTDecorator_TargetsKineticShieldAreDown(const FObjectInitializer& ObjectInitializer);
	
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;
};
