// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "RCombatScenarioSubsystem.generated.h"

class ARCombatScenario;
struct FCombatEnded;

/**
 * Manages combat scenarios — orchestrates pre/post combat phases
 * and bridges scenario actors with the CombatManager turn system.
 */
UCLASS()
class RIZZGAME_API URCombatScenarioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Scenario Registration (called by ARCombatScenario BeginPlay/EndPlay) ---

	void RegisterScenario(ARCombatScenario* Scenario);
	void UnregisterScenario(ARCombatScenario* Scenario);

	// --- Scenario Startup ---

	/** Start a combat scenario by its ScenarioId tag. Returns false if no matching scenario is found or a scenario is already active. */
	UFUNCTION(BlueprintCallable, Category="Combat Scenario")
	bool StartScenario(FGameplayTag ScenarioId);

	/** Start a combat scenario by direct reference. Returns false if the scenario is null or a scenario is already active. */
	UFUNCTION(BlueprintCallable, Category="Combat Scenario")
	bool StartScenarioByRef(ARCombatScenario* Scenario);

	// --- Queries ---

	/** Returns the currently active combat scenario, or null if none. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat Scenario")
	ARCombatScenario* GetActiveScenario() const;

	/** Returns true if a combat scenario is currently in progress. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat Scenario")
	bool IsInScenario() const;

	/** Find a registered scenario by its ScenarioId tag. Returns null if not found. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat Scenario")
	ARCombatScenario* FindScenario(FGameplayTag ScenarioId) const;

	// --- Internal Flow (called by ARCombatScenario) ---

	void OnPreCombatFinished(ARCombatScenario* Scenario);
	void OnPostCombatFinished(ARCombatScenario* Scenario);

private:
	UPROPERTY()
	TArray<TObjectPtr<ARCombatScenario>> RegisteredScenarios;

	UPROPERTY()
	TObjectPtr<ARCombatScenario> ActiveScenario;

	FDelegateHandle CombatEndedHandle;

	bool StartScenarioInternal(ARCombatScenario* Scenario);
	void BeginCombatForScenario(ARCombatScenario* Scenario);
	void HandleCombatEnded(const FCombatEnded& Event);
};
