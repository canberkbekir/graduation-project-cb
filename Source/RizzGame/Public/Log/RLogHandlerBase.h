// RLogHandlerBase.h
#pragma once

#include "CoreMinimal.h"
#include "Log/RLogEntry.h"
#include "RLogHandlerBase.generated.h"

/**
 * Handles formatting, filtering, and reacting to log entries.
 * Subclass in Blueprint or C++ to customize any of the three behaviours independently.
 */
UCLASS(Blueprintable, BlueprintType)
class RIZZGAME_API URLogHandlerBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Converts a log entry into display text.
	 * Default: "SourceActor → TargetActor: rolled [4, 3] +1 = 8"
	 * Override to change layout, add colour tags, localise strings, etc.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RizzGame|Log")
	FText FormatEntry(const FRLogEntry& Entry) const;
	virtual FText FormatEntry_Implementation(const FRLogEntry& Entry) const;

	/**
	 * Return false to drop this entry before it reaches the log.
	 * Default: always true (log everything).
	 * Override to filter by category, source, or any other field.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RizzGame|Log")
	bool ShouldLog(const FRLogEntry& Entry) const;
	virtual bool ShouldLog_Implementation(const FRLogEntry& Entry) const { return true; }

	/**
	 * Called after the entry has been stored and broadcast.
	 * Default: no-op.
	 * Override to play sounds, trigger UI animations, write to disk, etc.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RizzGame|Log")
	void OnEntryAdded(const FRLogEntry& Entry);
	virtual void OnEntryAdded_Implementation(const FRLogEntry& Entry) {}

	/**
	 * Formats a dice result into a readable roll breakdown string.
	 * e.g. FDiceRollResult{Total=8, IndividualRolls=[4,3], FlatModifier=1}
	 *   -> "(rolled [4, 3] +1 = 8)"
	 * Call this inside FormatEntry to append dice detail to any log line.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RizzGame|Log")
	static FText FormatDiceRoll(const FDiceRollResult& Result);
};
