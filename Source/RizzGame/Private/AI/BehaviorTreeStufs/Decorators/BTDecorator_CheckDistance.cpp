


#include "AI/BehaviorTreeStufs/Decorators/BTDecorator_CheckDistance.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_CheckDistance::UBTDecorator_CheckDistance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "CheckDistance";
	bNotifyTick = true;
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckDistance, TargetActorKey), AActor::StaticClass());
	DistanceThresholdKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckDistance, DistanceThresholdKey));
}

bool UBTDecorator_CheckDistance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!AIController || !BlackboardComp)
		return false;
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)
		return false;
	AActor* Owner = AIController->GetPawn();
	if (!Owner)
		return false;
	float DistanceSquared = FVector::DistSquared(Owner->GetActorLocation(), TargetActor->GetActorLocation());
	float DistanceThreshold = BlackboardComp->GetValueAsFloat(DistanceThresholdKey.SelectedKeyName);
	float ThresholdSquared = DistanceThreshold * DistanceThreshold;
	if (bCheckIfCloser)
		return DistanceSquared <= ThresholdSquared;
	return DistanceSquared > ThresholdSquared;
}

void UBTDecorator_CheckDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (!CalculateRawConditionValue(OwnerComp, NodeMemory))
	{
		OwnerComp.RequestExecution(this);
	}
}