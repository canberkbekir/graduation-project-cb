#pragma once

#include "GameplayEffectExecutionCalculation.h"
#include "RPhysicalDamageExecutionCalculation.generated.h"

UCLASS()
class RIZZGAME_API URPhysicalDamageExecutionCalculation
	: public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	URPhysicalDamageExecutionCalculation();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput
	) const override;
};
