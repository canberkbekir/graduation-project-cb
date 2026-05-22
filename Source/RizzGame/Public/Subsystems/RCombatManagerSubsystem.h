// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/RTurnComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "GameplayEffectTypes.h"
#include "Events/CombatEvents.h"
#include "RCombatManagerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStartedDelegate, FGameplayTag, EncounterId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatEndedDelegate, FGameplayTag, EncounterId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnStartedDelegate, FName, ActiveActorId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnEndedDelegate, FName, ActiveActorId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInitiativeChangedDelegate, FCombatInitiativeChanged, InitiativeData);

/**
 *
 */
UCLASS()
class RIZZGAME_API URCombatManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	void AddToCombatants(URTurnComponent* Combatant);

	UFUNCTION(BlueprintCallable)
	void RemoveFromCombat(URTurnComponent* Combatant);

	UFUNCTION(BlueprintCallable)
	void EnterCombat();// TODO: rename functions (StartTurnCycle)

	void AdvanceTurn();

	UFUNCTION(BlueprintCallable)
	void EndCombat();

	/** Removes all combatants from the active roster without triggering ExitCombat on them. */
	UFUNCTION(BlueprintCallable)
	void ClearCombatants();

	void CalculateInitiative();

	UPROPERTY(BlueprintAssignable, Category="Combat|Events")
	FOnCombatStartedDelegate OnCombatStarted;

	UPROPERTY(BlueprintAssignable, Category="Combat|Events")
	FOnCombatEndedDelegate OnCombatEnded;

	UPROPERTY(BlueprintAssignable, Category="Combat|Events")
	FOnTurnStartedDelegate OnTurnStarted;

	UPROPERTY(BlueprintAssignable, Category="Combat|Events")
	FOnTurnEndedDelegate OnTurnEnded;

	UPROPERTY(BlueprintAssignable, Category="Combat|Events")
	FOnInitiativeChangedDelegate OnInitiativeChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	TObjectPtr<UDataTable> StatusEffectDisplayTable;

	/** The encounter identifier set by the active combat scenario. */
	UPROPERTY(BlueprintReadOnly, Category="Combat")
	FGameplayTag ActiveEncounterId;

	UPROPERTY(BlueprintReadOnly)
	TArray<TObjectPtr<URTurnComponent>> Combatants;

	UPROPERTY(BlueprintReadOnly)
	uint8 CurrentCombatantIndex = -1;

private:
	int32 RoundNumber = 1;

	void PublishInitiativeEvent();

	void BindAttributeListeners();
	void UnbindAttributeListeners();

	void OnAttributeChanged(const FOnAttributeChangeData& Data);
	void OnEffectAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	void OnEffectRemoved(const FActiveGameplayEffect& RemovedEffect);

	TArray<FDelegateHandle> AttributeDelegateHandles;
	TArray<FDelegateHandle> EffectAddedHandles;
	TArray<FDelegateHandle> EffectRemovedHandles;
};
