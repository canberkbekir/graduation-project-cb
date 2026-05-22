

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckDistance.generated.h"


UCLASS()
class RIZZGAME_API UBTService_CheckDistance : public UBTService
{
	GENERATED_BODY()
public:
	UBTService_CheckDistance(const FObjectInitializer& ObjectInitializer);
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, Category = "Target")
	FBlackboardKeySelector TargetActorKey;
	
	UPROPERTY(EditAnywhere, Category = "Check 1")
	bool bCheckIfCloser1 = true;

	UPROPERTY(EditAnywhere, Category = "Check 1")
	FBlackboardKeySelector DistanceThresholdKey1;

	UPROPERTY(EditAnywhere, Category = "Check 1")
	FBlackboardKeySelector ResultBoolKey1;
	
	UPROPERTY(EditAnywhere, Category = "Check 2")
	bool bCheckIfCloser2 = true;

	UPROPERTY(EditAnywhere, Category = "Check 2")
	FBlackboardKeySelector DistanceThresholdKey2;

	UPROPERTY(EditAnywhere, Category = "Check 2")
	FBlackboardKeySelector ResultBoolKey2;
};
