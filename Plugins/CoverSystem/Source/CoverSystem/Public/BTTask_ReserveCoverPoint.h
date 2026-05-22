
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ReserveCoverPoint.generated.h"

UCLASS()
class COVERSYSTEM_API UBTTask_ReserveCoverPoint : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_ReserveCoverPoint(const FObjectInitializer& ObjectInitializer);
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "Cover")
	FBlackboardKeySelector CoverPointKey;
};
