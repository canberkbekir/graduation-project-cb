// RLogEntry.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/DiceSubsystem.h"
#include "RLogEntry.generated.h"

UENUM(BlueprintType)
enum class ERLogCategory : uint8
{
	Combat,
	Exploration,
	Social,
	System
};

USTRUCT(BlueprintType)
struct RIZZGAME_API FRLogEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly)
	ERLogCategory Category = ERLogCategory::System;

	/** Name of the actor who triggered this event. */
	UPROPERTY(BlueprintReadOnly)
	FName SourceActorId = NAME_None;

	/** Name of the actor who received this event. */
	UPROPERTY(BlueprintReadOnly)
	FName TargetActorId = NAME_None;

	/** Optional override message. If empty, URLogHandlerBase::FormatEntry builds one. */
	UPROPERTY(BlueprintReadWrite)
	FText Message;

	/** Damage/heal entries: the MagnitudeTag (e.g. Data.Damage.Physical).
	 *  Check entries: the CheckTag (e.g. Check.Hit, Check.Save.Physical). */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag EventTag;

	/** Check entries only: the Dodge value or AbilityDC the roll was compared against. */
	UPROPERTY(BlueprintReadOnly)
	int32 CheckTargetValue = 0;

	/** Check entries only: true when the roll met or exceeded the target (hit or save success). */
	UPROPERTY(BlueprintReadOnly)
	bool bCheckSuccess = false;

	/** Check entries only: true when the d20 landed on 20 (critical hit or critical save). */
	UPROPERTY(BlueprintReadOnly)
	bool bCheckCritical = false;

	UPROPERTY(BlueprintReadOnly)
	bool bHasDiceRoll = false;

	UPROPERTY(BlueprintReadOnly)
	FDiceExpression DiceExpression;

	UPROPERTY(BlueprintReadOnly)
	FDiceRollResult DiceResult;
};
