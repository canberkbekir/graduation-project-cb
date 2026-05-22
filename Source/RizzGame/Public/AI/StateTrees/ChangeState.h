#pragma once
#include "GameplayEffect.h"
#include "StateTreeTaskBase.h" 
#include "Components/RTurnComponent.h"
#include "ChangeState.generated.h"


USTRUCT()
struct FChangeStateData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> Actor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input")
	ETurnState TurnState;

	UPROPERTY(EditAnywhere, Category = "Input")
	TSubclassOf<UGameplayEffect> Effect;
};

USTRUCT(BlueprintType)
struct RIZZGAME_API FChangeState : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FChangeState() = default;
	virtual const UStruct* GetInstanceDataType() const override { return FChangeStateData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
	                                       const FStateTreeTransitionResult& Transition) const override;
};
