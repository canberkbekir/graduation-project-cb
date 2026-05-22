


#include "FStateTreeTasks_CoverSystem.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FStateTreeTask_ReserveCoverPoint::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTask_ReserveCoverPointInstanceData* InstanceData = Context.GetInstanceDataPtr<FStateTreeTask_ReserveCoverPointInstanceData>(*this);
	if (!InstanceData || !InstanceData->Actor || !InstanceData->CoverPoint)
		return EStateTreeRunStatus::Failed;
	AActor* Actor = InstanceData->Actor;
	UCoverComponent* CoverComponent = Actor->FindComponentByClass<UCoverComponent>();
	if (!CoverComponent)
		return EStateTreeRunStatus::Failed;
	ACoverPoint* CoverPoint = Cast<ACoverPoint>(InstanceData->CoverPoint);
	if (!CoverPoint || !CoverPoint->IsAvailable())
		return EStateTreeRunStatus::Failed;
	CoverComponent->ReserveCoverPoint(CoverPoint);
	return EStateTreeRunStatus::Succeeded;
}


EStateTreeRunStatus FStateTreeTask_OccupyCoverPoint::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTask_OccupyCoverPointInstanceData* InstanceData = Context.GetInstanceDataPtr<FStateTreeTask_OccupyCoverPointInstanceData>(*this);
	if (!InstanceData || !InstanceData->Actor || !InstanceData->CoverPoint)
		return EStateTreeRunStatus::Failed;
	AActor* Actor = InstanceData->Actor;
	UCoverComponent* CoverComponent = Actor->FindComponentByClass<UCoverComponent>();
	if (!CoverComponent)	
		return EStateTreeRunStatus::Failed;
	ACoverPoint* CoverPoint = Cast<ACoverPoint>(InstanceData->CoverPoint);
	if (!CoverPoint)	
		return EStateTreeRunStatus::Failed;
	if (InstanceData->bIsReservedBeforeOccupying && CoverPoint->IsReservedBy(Actor))
	{
		CoverComponent->OccupyCoverPoint(CoverPoint, InstanceData->bIsPhysicllyReached);
		return EStateTreeRunStatus::Succeeded;
	}
	if (CoverPoint->IsAvailable())
	{
		CoverComponent->OccupyCoverPointWithoutReservation(CoverPoint, InstanceData->bIsPhysicllyReached);
		return EStateTreeRunStatus::Succeeded;
	}
	return EStateTreeRunStatus::Failed;
}


EStateTreeRunStatus FStateTreeTask_FreeCoverPoint::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTask_FreeCoverPointInstanceData* InstanceData = Context.GetInstanceDataPtr<FStateTreeTask_FreeCoverPointInstanceData>(*this);
	if (!InstanceData || !InstanceData->Actor)
		return EStateTreeRunStatus::Failed;
	AActor* Actor = InstanceData->Actor;
	UCoverComponent* CoverComponent = Actor->FindComponentByClass<UCoverComponent>();
	if (!CoverComponent)
		return EStateTreeRunStatus::Failed;
	CoverComponent->ClearCoverPoint();
	return EStateTreeRunStatus::Succeeded;
}



EStateTreeRunStatus FStateTreeTask_OutFromCoverPoint::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTask_OutFromCoverPointInstanceData* InstanceData = Context.GetInstanceDataPtr<FStateTreeTask_OutFromCoverPointInstanceData>(*this);
	if (!InstanceData || !InstanceData->Actor)
		return EStateTreeRunStatus::Failed;
	AActor* Actor = InstanceData->Actor;
	UCoverComponent* CoverComponent = Actor->FindComponentByClass<UCoverComponent>();
	if (!CoverComponent)
		return EStateTreeRunStatus::Failed;
	if (CoverComponent->OutFromCover())
		return EStateTreeRunStatus::Succeeded;
	return EStateTreeRunStatus::Failed;
}


EStateTreeRunStatus FStateTreeTask_BackToCover::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTask_BackToCoverInstanceData* InstanceData = Context.GetInstanceDataPtr<FStateTreeTask_BackToCoverInstanceData>(*this);
	if (!InstanceData || !InstanceData->Actor)	
		return EStateTreeRunStatus::Failed;
	AActor* Actor = InstanceData->Actor;
	UCoverComponent* CoverComponent = Actor->FindComponentByClass<UCoverComponent>();
	if (!CoverComponent)	
		return EStateTreeRunStatus::Failed;
	if (CoverComponent->BackToCover())
		return EStateTreeRunStatus::Succeeded;
	return EStateTreeRunStatus::Failed;
}


EStateTreeRunStatus FStateTreeTask_SetIsInCover::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTask_SetIsInCoverInstanceData* InstanceData = Context.GetInstanceDataPtr<FStateTreeTask_SetIsInCoverInstanceData>(*this);
	if (!InstanceData || !InstanceData->Actor)	
		return EStateTreeRunStatus::Failed;
	AActor* Actor = InstanceData->Actor;
	UCoverComponent* CoverComponent = Actor->FindComponentByClass<UCoverComponent>();
	if (!CoverComponent)	
		return EStateTreeRunStatus::Failed;
	CoverComponent->SetIsInCover(InstanceData->bIsInCover);
	return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FStateTreeTask_SetCoverPosition::EnterState(FStateTreeExecutionContext& Context,const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTask_SetCoverPositionInstanceData* InstanceData = Context.GetInstanceDataPtr<FStateTreeTask_SetCoverPositionInstanceData>(*this);
	if (!InstanceData || !InstanceData->Actor)	
		return EStateTreeRunStatus::Failed;
	AActor* Actor = InstanceData->Actor;
	UCoverComponent* CoverComponent = Actor->FindComponentByClass<UCoverComponent>();
	if (!CoverComponent)	
		return EStateTreeRunStatus::Failed;
	auto coverType = CoverComponent->GetCoverType();
	switch (coverType)
	{
		case ECoverType::CrouchCover:
			CoverComponent->SetCrouchCoverPosition(InstanceData->CrouchPosition);
			return EStateTreeRunStatus::Succeeded;
		case ECoverType::StandCover:
			CoverComponent->SetStandCoverPosition(InstanceData->StandPosition);
			return EStateTreeRunStatus::Succeeded;
	}
	return EStateTreeRunStatus::Failed;
}
