

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetIsInCover.generated.h"

UCLASS()
class COVERSYSTEM_API UBTTask_SetIsInCover : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_SetIsInCover(const FObjectInitializer& ObjectInitializer);
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "Cover")
	bool bIsInCover = true;
};
