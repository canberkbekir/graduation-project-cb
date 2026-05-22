

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BackToCover.generated.h"

UCLASS()
class COVERSYSTEM_API UBTTask_BackToCover : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_BackToCover(const FObjectInitializer& ObjectInitializer);
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
