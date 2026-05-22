// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DiceSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDiceType : uint8
{
	D4,
	D6,
	D8,
	D10,
	D12,
	D20
};

UENUM(BlueprintType)
enum class EDiceRollContext : uint8
{
	None,
	Combat,
	Exploration,
	Social
};

USTRUCT(BlueprintType)
struct RIZZGAME_API FDiceTerm
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1"))
	int32 Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDiceType Die = EDiceType::D6;
};

USTRUCT(BlueprintType)
struct RIZZGAME_API FDiceExpression
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDiceTerm> Terms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FlatModifier = 0;

	bool IsEmpty() const { return Terms.Num() == 0 && FlatModifier == 0; }
};

USTRUCT(BlueprintType)
struct RIZZGAME_API FDiceRollResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Total = 0;

	/** One value per die rolled, in term order. */
	UPROPERTY(BlueprintReadOnly)
	TArray<int32> IndividualRolls;

	UPROPERTY(BlueprintReadOnly)
	int32 FlatModifier = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDiceRolled, const FDiceExpression&, Expression, const FDiceRollResult&, Result, EDiceRollContext, RollContext);

/**
 *
 */
UCLASS()
class RIZZGAME_API UDiceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Bind in Blueprint to react to any dice roll. */
	UPROPERTY(BlueprintAssignable, Category = "RizzGame|Dice")
	FOnDiceRolled OnDiceRolled;

	UFUNCTION(BlueprintCallable, Category = "RizzGame")
	int32 RollDice(EDiceType Type, uint8 Count) const;

	UFUNCTION(BlueprintCallable, Category = "RizzGame|Dice")
	FDiceRollResult RollExpression(const FDiceExpression& Expression, EDiceRollContext RollContext) const;

	UFUNCTION(BlueprintCallable, Category = "RizzGame|Dice")
	FDiceRollResult RollNotation(const FString& Notation, EDiceRollContext RollContext) const;

	/** Parses notation like "2d6+1d4+1". Only '+' is supported; '-' returns false. */
	UFUNCTION(BlueprintCallable, Category = "RizzGame|Dice", meta=(DisplayName="Parse Dice Notation"))
	static bool ParseDiceNotation(const FString& Notation, FDiceExpression& OutExpression);

private:
	int32 GetDiceSides(EDiceType Type) const;
	static bool SidesToDiceType(int32 Sides, EDiceType& OutType);
	FRandomStream RNG;
};
