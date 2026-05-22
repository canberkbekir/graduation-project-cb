
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_WalkDistanceFinished.generated.h"

UCLASS()
class RIZZGAME_API UBTDecorator_WalkDistanceFinished : public UBTDecorator
{
	GENERATED_BODY()
public:
	UBTDecorator_WalkDistanceFinished(const FObjectInitializer& ObjectInitializer);
	
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

};
