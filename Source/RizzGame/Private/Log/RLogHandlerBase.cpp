// RLogHandlerBase.cpp
#include "Log/RLogHandlerBase.h"

FText URLogHandlerBase::FormatEntry_Implementation(const FRLogEntry& Entry) const
{
	if (!Entry.Message.IsEmpty())
	{
		return Entry.Message;
	}

	FString Out;

	if (Entry.SourceActorId != NAME_None)
	{
		Out += Entry.SourceActorId.ToString();
		if (Entry.TargetActorId != NAME_None)
		{
			Out += TEXT(" -> ") + Entry.TargetActorId.ToString();
		}
		Out += TEXT(": ");
	}

	if (Entry.bHasDiceRoll)
	{
		FString Rolls = TEXT("[");
		for (int32 i = 0; i < Entry.DiceResult.IndividualRolls.Num(); ++i)
		{
			if (i > 0) Rolls += TEXT(", ");
			Rolls += FString::FromInt(Entry.DiceResult.IndividualRolls[i]);
		}
		Rolls += TEXT("]");

		if (Entry.DiceResult.FlatModifier != 0)
		{
			Rolls += FString::Printf(TEXT(" +%d"), Entry.DiceResult.FlatModifier);
		}

		Out += FString::Printf(TEXT("rolled %s = %d"), *Rolls, Entry.DiceResult.Total);
	}

	return FText::FromString(Out.IsEmpty() ? TEXT("(no data)") : Out);
}

FText URLogHandlerBase::FormatDiceRoll(const FDiceRollResult& Result)
{
	if (Result.IndividualRolls.Num() == 0)
	{
		return FText::GetEmpty();
	}

	FString Rolls = TEXT("[");
	for (int32 i = 0; i < Result.IndividualRolls.Num(); ++i)
	{
		if (i > 0) Rolls += TEXT(", ");
		Rolls += FString::FromInt(Result.IndividualRolls[i]);
	}
	Rolls += TEXT("]");

	if (Result.FlatModifier != 0)
	{
		Rolls += FString::Printf(TEXT(" +%d"), Result.FlatModifier);
	}

	return FText::FromString(FString::Printf(TEXT("(rolled %s = %d)"), *Rolls, Result.Total));
}
