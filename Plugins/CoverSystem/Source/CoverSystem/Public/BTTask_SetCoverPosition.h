

#pragma once

#include "CoreMinimal.h"
#include "CoverEnumsStructs.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetCoverPosition.generated.h"

UCLASS()
class COVERSYSTEM_API UBTTask_SetCoverPosition : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_SetCoverPosition(const FObjectInitializer& ObjectInitializer);
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Cover")
	ECrouchCoverPositions CrouchPosition = ECrouchCoverPositions::Idle;
	UPROPERTY(EditAnywhere, Category = "Cover")
	EStandCoverPositions StandPosition = EStandCoverPositions::Idle;
	
};
