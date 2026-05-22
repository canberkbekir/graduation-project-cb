// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/DiceSubsystem.h"
#include "Events/DiceEvents.h"
#include "Subsystems/REventBusSubsystem.h"

void UDiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RNG.Initialize(FDateTime::Now().GetTicks());

	// deterministic (debug / replay)
	// RNG.Initialize(123456);
}

int32 UDiceSubsystem::RollDice(const EDiceType Type, const uint8 Count) const
{
	const int32 Sides = GetDiceSides(Type);
	int32 Total = 0;

	for (int32 i = 0; i < Count; ++i)
	{
		Total += RNG.RandRange(1, Sides);
	}

	return Total;
}

FDiceRollResult UDiceSubsystem::RollExpression(const FDiceExpression& Expression, const EDiceRollContext RollContext) const
{
	FDiceRollResult Result;
	Result.FlatModifier = Expression.FlatModifier;

	for (const FDiceTerm& Term : Expression.Terms)
	{
		const int32 Sides = GetDiceSides(Term.Die);
		for (int32 i = 0; i < Term.Count; ++i)
		{
			const int32 Roll = RNG.RandRange(1, Sides);
			Result.IndividualRolls.Add(Roll);
			Result.Total += Roll;
		}
	}

	Result.Total += Expression.FlatModifier;

	if (UREventBusSubsystem* EB = GetGameInstance()->GetSubsystem<UREventBusSubsystem>())
	{
		FDiceRolled Event;
		Event.Expression = Expression;
		Event.Result = Result;
		Event.RollContext = RollContext;
		EB->Publish(Event);
	}

	OnDiceRolled.Broadcast(Expression, Result, RollContext);

	return Result;
}

FDiceRollResult UDiceSubsystem::RollNotation(const FString& Notation, const EDiceRollContext RollContext) const
{
	FDiceExpression Expression;
	if (!ParseDiceNotation(Notation, Expression))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDiceSubsystem::RollNotation — failed to parse '%s'"), *Notation);
		return FDiceRollResult{};
	}
	return RollExpression(Expression, RollContext);
}

bool UDiceSubsystem::ParseDiceNotation(const FString& Notation, FDiceExpression& OutExpression)
{
	OutExpression = FDiceExpression{};

	FString Cleaned = Notation.Replace(TEXT(" "), TEXT("")).Replace(TEXT("\t"), TEXT(""));
	if (Cleaned.IsEmpty())
	{
		return false;
	}

	// '-' operator not supported in v1
	if (Cleaned.Contains(TEXT("-")))
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseDiceNotation: '-' operator not supported in '%s'"), *Notation);
		return false;
	}

	TArray<FString> Tokens;
	Cleaned.ParseIntoArray(Tokens, TEXT("+"), true);

	if (Tokens.Num() == 0)
	{
		return false;
	}

	for (const FString& Token : Tokens)
	{
		int32 DPos = INDEX_NONE;
		Token.ToLower().FindChar(TEXT('d'), DPos);

		if (DPos != INDEX_NONE)
		{
			const FString CountStr = Token.Left(DPos);
			const FString SidesStr = Token.Mid(DPos + 1);

			if (SidesStr.IsEmpty() || !SidesStr.IsNumeric())
			{
				return false;
			}
			if (!CountStr.IsEmpty() && !CountStr.IsNumeric())
			{
				return false;
			}

			const int32 Count = CountStr.IsEmpty() ? 1 : FCString::Atoi(*CountStr);
			const int32 Sides = FCString::Atoi(*SidesStr);

			if (Count < 1 || Sides <= 0)
			{
				return false;
			}

			EDiceType DieType;
			if (!SidesToDiceType(Sides, DieType))
			{
				UE_LOG(LogTemp, Warning, TEXT("ParseDiceNotation: unsupported die size d%d in '%s'"), Sides, *Notation);
				return false;
			}

			FDiceTerm Term;
			Term.Count = Count;
			Term.Die = DieType;
			OutExpression.Terms.Add(Term);
		}
		else
		{
			if (!Token.IsNumeric())
			{
				return false;
			}
			OutExpression.FlatModifier += FCString::Atoi(*Token);
		}
	}

	return true;
}

int32 UDiceSubsystem::GetDiceSides(const EDiceType Type) const
{
	switch (Type)
	{
	case EDiceType::D4:  return 4;
	case EDiceType::D6:  return 6;
	case EDiceType::D8:  return 8;
	case EDiceType::D10: return 10;
	case EDiceType::D12: return 12;
	case EDiceType::D20: return 20;
	default: return 0;
	}
}

bool UDiceSubsystem::SidesToDiceType(const int32 Sides, EDiceType& OutType)
{
	switch (Sides)
	{
	case 4:  OutType = EDiceType::D4;  return true;
	case 6:  OutType = EDiceType::D6;  return true;
	case 8:  OutType = EDiceType::D8;  return true;
	case 10: OutType = EDiceType::D10; return true;
	case 12: OutType = EDiceType::D12; return true;
	case 20: OutType = EDiceType::D20; return true;
	default: return false;
	}
}
