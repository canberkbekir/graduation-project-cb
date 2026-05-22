

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_EndTurn.generated.h"


UCLASS()
class RIZZGAME_API UBTTask_EndTurn : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_EndTurn(const FObjectInitializer& InitializerModule);
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category="Blackboard")
	bool bClearFocus = true;
};
