

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_ShieldsAreDown.generated.h"

UCLASS()
class RIZZGAME_API UBTDecorator_ShieldsAreDown : public UBTDecorator
{
	GENERATED_BODY()
public:
	UBTDecorator_ShieldsAreDown(const FObjectInitializer& InitializerModule);
	
	UPROPERTY(EditAnywhere)
	bool bIsKineticShield = true;
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
