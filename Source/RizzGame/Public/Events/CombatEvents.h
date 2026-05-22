// CombatEvents.h
#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/RTurnComponent.h"
#include "UI/Models/RCombatUIModels.h"
#include "CombatEvents.generated.h"

USTRUCT(BlueprintType)
struct FCombatStarted
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag EncounterId;
};

USTRUCT(BlueprintType)
struct FCombatEnded
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag EncounterId;
};

USTRUCT(BlueprintType)
struct FTurnStarted
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	FName ActiveActorId = NAME_None;
};

USTRUCT(BlueprintType)
struct FTurnEnded
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	FName ActiveActorId = NAME_None;
};

USTRUCT(BlueprintType)
struct FInitiativeCombatantInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName ActorId = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	ERCombatTeam Team = ERCombatTeam::Enemy;

	UPROPERTY(BlueprintReadOnly)
	UTexture2D* Portrait = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentHealth = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxHealth = 1;

	UPROPERTY(BlueprintReadOnly)
	float CurrentKineticShields = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float MaxKineticShields = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float CurrentEnergyShields = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float MaxEnergyShields = 0.f;

	UPROPERTY(BlueprintReadOnly)
	TArray<FEffectIconViewModel> StatusEffects;
};

/** Broadcast when an ability is granted to a character */
USTRUCT()
struct FAbilityGranted
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag AbilityTag;

	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;
};

/** Broadcast when an ability is removed from a character */
USTRUCT()
struct FAbilityRemoved
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag AbilityTag;

	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;
};

USTRUCT(BlueprintType)
struct FCombatInitiativeChanged
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FInitiativeCombatantInfo> Combatants;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentTurnIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 RoundNumber = 1;
};
