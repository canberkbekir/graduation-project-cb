

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckTargetHealth.generated.h"


UCLASS()
class RIZZGAME_API UBTDecorator_CheckTargetHealth : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CheckTargetHealth(const FObjectInitializer& InitializerModule);
	
	UPROPERTY(EditAnywhere, Category = "Health", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float HealthPertange;
	UPROPERTY(EditAnywhere, Category = "Health")
	bool bBelow = false;
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
