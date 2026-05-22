// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/DiceSubsystem.h"
#include "RCombatTypes.generated.h"

UENUM(BlueprintType)
enum class EResistanceType : uint8
{
	Physical        UMETA(DisplayName="Physical"),
	Network         UMETA(DisplayName="Network"),
	Mental          UMETA(DisplayName="Mental"),
	Radiation       UMETA(DisplayName="Radiation"),
	Electromagnetic UMETA(DisplayName="Electromagnetic"),
};

UENUM(BlueprintType)
enum class EHitOutcome : uint8
{
	Miss        UMETA(DisplayName="Miss"),
	Hit         UMETA(DisplayName="Hit"),
	CriticalHit UMETA(DisplayName="Critical Hit"),
};

UENUM(BlueprintType)
enum class ESaveOutcome : uint8
{
	Fail            UMETA(DisplayName="Fail"),
	Success         UMETA(DisplayName="Success"),
	CriticalSuccess UMETA(DisplayName="Critical Success"),
};

USTRUCT(BlueprintType)
struct FHitCheckResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EHitOutcome Outcome = EHitOutcome::Miss;

	UPROPERTY(BlueprintReadOnly)
	FDiceRollResult Roll;

	/** Dodge value (10 + FinesseMod) of the target that the attack roll was compared against. */
	UPROPERTY(BlueprintReadOnly)
	int32 TargetDodge = 0;
};

USTRUCT(BlueprintType)
struct FSaveCheckResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	ESaveOutcome Outcome = ESaveOutcome::Fail;

	UPROPERTY(BlueprintReadOnly)
	FDiceRollResult Roll;

	/** Ability DC the save roll was compared against. */
	UPROPERTY(BlueprintReadOnly)
	int32 AbilityDC = 0;
};
