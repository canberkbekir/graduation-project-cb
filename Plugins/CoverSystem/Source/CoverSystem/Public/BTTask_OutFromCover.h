

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_OutFromCover.generated.h"

UCLASS()
class COVERSYSTEM_API UBTTask_OutFromCover : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_OutFromCover(const FObjectInitializer& ObjectInitializer);
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
