// RLogSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Events/AbilityEvents.h"
#include "Events/CheckEvents.h"
#include "Log/RLogEntry.h"
#include "Log/RLogHandlerBase.h"
#include "RLogSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLogEntryAdded, const FRLogEntry&, Entry);

/**
 * Stores the game-wide event log and routes entries through a swappable handler.
 * Call SetHandlerClass() at runtime (e.g. from GameMode) to plug in a custom handler.
 * Bind OnLogEntryAdded in Blueprint or C++ to update UI whenever a new entry arrives.
 */
UCLASS()
class RIZZGAME_API URLogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── Handler ────────────────────────────────────────────────────────────────

	/**
	 * Swap the active handler at runtime.
	 * Pass nullptr to revert to the default URLogHandlerBase.
	 */
	UFUNCTION(BlueprintCallable, Category = "RizzGame|Log")
	void SetHandlerClass(TSubclassOf<URLogHandlerBase> NewHandlerClass);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RizzGame|Log")
	URLogHandlerBase* GetHandler() const { return Handler; }

	// ── Entry management ──────────────────────────────────────────────────────

	/**
	 * Push a new entry through the handler pipeline.
	 * ShouldLog is checked first; if false the entry is silently dropped.
	 * Timestamp is stamped here if it hasn't been set by the caller.
	 */
	UFUNCTION(BlueprintCallable, Category = "RizzGame|Log")
	void AddEntry(FRLogEntry Entry);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RizzGame|Log")
	const TArray<FRLogEntry>& GetEntries() const { return Entries; }

	UFUNCTION(BlueprintCallable, Category = "RizzGame|Log")
	void ClearEntries() { Entries.Reset(); }

	// ── Event ─────────────────────────────────────────────────────────────────

	/** Fired after every accepted entry is stored. Bind this to drive your log UI. */
	UPROPERTY(BlueprintAssignable, Category = "RizzGame|Log")
	FOnLogEntryAdded OnLogEntryAdded;

private:
	UPROPERTY()
	TObjectPtr<URLogHandlerBase> Handler;

	TArray<FRLogEntry> Entries;

	FDelegateHandle AbilityResolvedHandle;
	FDelegateHandle CheckResolvedHandle;

	void OnAbilityResolved(const FAbilityResolved& Event);
	void OnCheckResolved(const FCheckResolved& Event);
};
