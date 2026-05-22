

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_OccupyCoverPoint.generated.h"

UCLASS()
class COVERSYSTEM_API UBTTask_OccupyCoverPoint : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_OccupyCoverPoint(const FObjectInitializer& ObjectInitializer);
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "Cover")
	FBlackboardKeySelector CoverPointKey;
	UPROPERTY(EditAnywhere, Category = "Cover")
	bool bIsPhysicllyReached = true;
	UPROPERTY(EditAnywhere, Category = "Cover")
	bool bIsReservedBeforeOccupying = false;
};
