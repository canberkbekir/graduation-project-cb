// DiceEvents.h
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/DiceSubsystem.h"
#include "DiceEvents.generated.h"

/** Broadcast every time any dice expression is rolled via UDiceSubsystem::RollExpression. */
USTRUCT(BlueprintType)
struct FDiceRolled
{
	GENERATED_BODY()

	/** What was rolled (terms + flat modifier). */
	UPROPERTY(BlueprintReadOnly)
	FDiceExpression Expression;

	UPROPERTY(BlueprintReadOnly)
	FDiceRollResult Result;

	UPROPERTY(BlueprintReadOnly)
	EDiceRollContext RollContext = EDiceRollContext::None;
};
