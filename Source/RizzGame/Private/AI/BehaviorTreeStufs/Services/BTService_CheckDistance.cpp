


#include "AI/BehaviorTreeStufs/Services/BTService_CheckDistance.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_CheckDistance::UBTService_CheckDistance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CheckDistance, TargetActorKey), AActor::StaticClass());
    
	DistanceThresholdKey1.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CheckDistance, DistanceThresholdKey1));
	ResultBoolKey1.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CheckDistance, ResultBoolKey1));
    
	DistanceThresholdKey2.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CheckDistance, DistanceThresholdKey2));
	ResultBoolKey2.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CheckDistance, ResultBoolKey2));
}

void UBTService_CheckDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();

	if (!BlackboardComp || !AIController || !AIController->GetPawn()) 
		return;

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)
	{
		if (!ResultBoolKey1.SelectedKeyName.IsNone())
			BlackboardComp->SetValueAsBool(ResultBoolKey1.SelectedKeyName, false);
		if (!ResultBoolKey2.SelectedKeyName.IsNone()) 
			BlackboardComp->SetValueAsBool(ResultBoolKey2.SelectedKeyName, false);
		return;
	}
	
	float DistanceSquared = FVector::DistSquared(AIController->GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());
	
	if (!ResultBoolKey1.SelectedKeyName.IsNone() && !DistanceThresholdKey1.SelectedKeyName.IsNone())
	{
		float Threshold1 = BlackboardComp->GetValueAsFloat(DistanceThresholdKey1.SelectedKeyName);
		float ThresholdSquared1 = Threshold1 * Threshold1;
		bool bConditionMet1 = bCheckIfCloser1 ? (DistanceSquared <= ThresholdSquared1) : (DistanceSquared > ThresholdSquared1);
		BlackboardComp->SetValueAsBool(ResultBoolKey1.SelectedKeyName, bConditionMet1);
	}
	
	if (!ResultBoolKey2.SelectedKeyName.IsNone() && !DistanceThresholdKey2.SelectedKeyName.IsNone())
	{
		float Threshold2 = BlackboardComp->GetValueAsFloat(DistanceThresholdKey2.SelectedKeyName);
		float ThresholdSquared2 = Threshold2 * Threshold2;
		bool bConditionMet2 = bCheckIfCloser2 ? (DistanceSquared <= ThresholdSquared2) : (DistanceSquared > ThresholdSquared2);
		BlackboardComp->SetValueAsBool(ResultBoolKey2.SelectedKeyName, bConditionMet2);
	}
}
