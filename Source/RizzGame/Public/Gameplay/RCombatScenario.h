// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "RCombatScenario.generated.h"

class ARCharacterBase;
class UBillboardComponent;
class USceneComponent;

UENUM(BlueprintType)
enum class ERCombatScenarioState : uint8
{
	Idle,
	PreCombat,
	InCombat,
	PostCombat,
	Completed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreCombatDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatStartDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPostCombatDelegate);

UCLASS()
class RIZZGAME_API ARCombatScenario : public AActor
{
	GENERATED_BODY()

public:
	ARCombatScenario();

	// --- Identity ---

	/** Unique tag identifier for this scenario. Select from the gameplay tag hierarchy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Scenario")
	FGameplayTag ScenarioId;

	// --- Enemy References ---

	/** Characters that will participate as enemies in this combat scenario. Pick actors placed in the level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Scenario")
	TArray<TObjectPtr<ARCharacterBase>> Enemies;

	// --- Configuration ---

	/** If true, all party members automatically join this combat. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Scenario|Config")
	bool bAutoJoinParty = true;

	/** If true, this scenario can be triggered again after completion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Scenario|Config")
	bool bRepeatable = false;

	/** If true, ExecutePreCombat runs before combat begins. Call FinishPreCombat() from Blueprint when ready. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Scenario|Config")
	bool bUsePreCombatPhase = false;

	/** If true, ExecutePostCombat runs after combat ends. Call FinishPostCombat() from Blueprint when ready. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Scenario|Config")
	bool bUsePostCombatPhase = false;

	// --- State ---

	/** Current state of this combat scenario. */
	UPROPERTY(BlueprintReadOnly, Category="Combat Scenario")
	ERCombatScenarioState ScenarioState = ERCombatScenarioState::Idle;

	// --- Delegates ---

	/** Broadcast when the pre-combat phase begins. */
	UPROPERTY(BlueprintAssignable, Category="Combat Scenario|Events")
	FOnPreCombatDelegate OnPreCombatDelegate;

	/** Broadcast when combat actually starts. */
	UPROPERTY(BlueprintAssignable, Category="Combat Scenario|Events")
	FOnCombatStartDelegate OnCombatStartDelegate;

	/** Broadcast when the post-combat phase begins. */
	UPROPERTY(BlueprintAssignable, Category="Combat Scenario|Events")
	FOnPostCombatDelegate OnPostCombatDelegate;

	// --- BlueprintImplementableEvents ---

	/** Override in Blueprint to run custom pre-combat logic (cinematics, dialogue, etc.). */
	UFUNCTION(BlueprintImplementableEvent, Category="Combat Scenario|Events")
	void OnPreCombat();

	/** Override in Blueprint to react when combat starts. */
	UFUNCTION(BlueprintImplementableEvent, Category="Combat Scenario|Events")
	void OnCombatStart();

	/** Override in Blueprint to run custom post-combat logic (loot, quests, etc.). */
	UFUNCTION(BlueprintImplementableEvent, Category="Combat Scenario|Events")
	void OnPostCombat();

	// --- Flow Control (called by Subsystem) ---

	void ExecutePreCombat();
	void ExecuteStartCombat();
	void ExecutePostCombat();

	// --- Blueprint Callable ---

	/** Call this from Blueprint when your pre-combat sequence (cinematics, dialogue) is finished. */
	UFUNCTION(BlueprintCallable, Category="Combat Scenario")
	void FinishPreCombat();

	/** Call this from Blueprint when your post-combat sequence (loot, quests) is finished. */
	UFUNCTION(BlueprintCallable, Category="Combat Scenario")
	void FinishPostCombat();

	void MarkCompleted();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<UBillboardComponent> SpriteComponent;
#endif

#if WITH_EDITOR
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }
#endif
};
