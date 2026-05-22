
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FreeCoverPoint.generated.h"

UCLASS()
class COVERSYSTEM_API UBTTask_FreeCoverPoint : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_FreeCoverPoint(const FObjectInitializer& ObjectInitializer);	
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
