
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckWhoAttackLastlyToTheTarget.generated.h"


UCLASS()
class RIZZGAME_API UBTDecorator_CheckWhoAttackLastlyToTheTarget : public UBTDecorator
{
	GENERATED_BODY()
public:
	UBTDecorator_CheckWhoAttackLastlyToTheTarget(const FObjectInitializer& InitializerModule);
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
protected:	
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
