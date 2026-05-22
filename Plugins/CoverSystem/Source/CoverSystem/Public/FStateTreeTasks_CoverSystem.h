#pragma once
#include "CoverEnumsStructs.h"
#include "StateTreeTaskBase.h"
#include "CoverComponent.h"
#include "FStateTreeTasks_CoverSystem.generated.h"

USTRUCT()
struct FStateTreeTask_ReserveCoverPointInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<AActor> CoverPoint;
};

USTRUCT(meta = (DisplayName = "Reserve Cover Point"))
struct FStateTreeTask_ReserveCoverPoint : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeTask_ReserveCoverPointInstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,const FStateTreeTransitionResult& Transition) const override;
};

USTRUCT()
struct FStateTreeTask_OccupyCoverPointInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<AActor> CoverPoint;
	UPROPERTY(EditAnywhere, Category = "Cover")
	bool bIsPhysicllyReached = true;
	UPROPERTY(EditAnywhere, Category = "Cover")
	bool bIsReservedBeforeOccupying = false;
	
};
USTRUCT(meta = (DisplayName = "Occupy Cover Point"))
struct FStateTreeTask_OccupyCoverPoint : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FStateTreeTask_OccupyCoverPointInstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,const FStateTreeTransitionResult& Transition) const override;
};

USTRUCT()
struct FStateTreeTask_FreeCoverPointInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;
};
USTRUCT(meta = (DisplayName = "Free Cover Point"))
struct FStateTreeTask_FreeCoverPoint : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FStateTreeTask_FreeCoverPointInstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,const FStateTreeTransitionResult& Transition) const override;
};

USTRUCT()
struct FStateTreeTask_OutFromCoverPointInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;
};
USTRUCT(meta = (DisplayName = "Out From Cover Point"))
struct FStateTreeTask_OutFromCoverPoint : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FStateTreeTask_OutFromCoverPointInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,const FStateTreeTransitionResult& Transition) const override;
};

USTRUCT()
struct FStateTreeTask_BackToCoverInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;
};
USTRUCT(meta = (DisplayName = "Back To Cover"))
struct FStateTreeTask_BackToCover : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FStateTreeTask_BackToCoverInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,const FStateTreeTransitionResult& Transition) const override;
};

USTRUCT()
struct FStateTreeTask_SetIsInCoverInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;
	UPROPERTY(EditAnywhere, Category = "Cover")
	bool bIsInCover = true;
};
USTRUCT(meta = (DisplayName = "Set Is In Cover"))
struct FStateTreeTask_SetIsInCover : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FStateTreeTask_SetIsInCoverInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,const FStateTreeTransitionResult& Transition) const override;
};

USTRUCT()
struct FStateTreeTask_SetCoverPositionInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;
	UPROPERTY(EditAnywhere, Category = "Cover")
	ECrouchCoverPositions CrouchPosition = ECrouchCoverPositions::Idle;
	UPROPERTY(EditAnywhere, Category = "Cover")
	EStandCoverPositions StandPosition = EStandCoverPositions::Idle;
};
USTRUCT(meta = (DisplayName = "Set Cover Position"))
struct FStateTreeTask_SetCoverPosition : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	using FInstanceDataType = FStateTreeTask_SetCoverPositionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,const FStateTreeTransitionResult& Transition) const override;
};
