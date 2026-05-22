#pragma once

#include "GameplayEffectExecutionCalculation.h"
#include "RNetworkDamageExecutionCalculation.generated.h"

UCLASS()
class RIZZGAME_API URNetworkDamageExecutionCalculation
	: public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	URNetworkDamageExecutionCalculation();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput
	) const override;
};
