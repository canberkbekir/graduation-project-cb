// ==== RCharacterSpec.h ====
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "ScalableFloat.h"
#include "RCharacterSpec.generated.h"

class UGameplayAbility;
class USkeletalMesh;
class UAnimInstance;
class UTexture2D;

/**
 * Single DataTable row containing all character definition data.
 * Designers edit one row per character with stats, abilities, visuals, etc.
 */
USTRUCT(BlueprintType)
struct RIZZGAME_API FCharacterDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	// ==================== VISUAL ====================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Visual")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Visual")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Visual")
	TSoftClassPtr<UAnimInstance> AnimClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Visual")
	TSoftObjectPtr<UTexture2D> Portrait;

	// ==================== CORE STATS ====================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Core")
	float Tech = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Core")
	float Physique = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Core")
	float Finesse = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Core")
	float Mind = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Core")
	float Insight = 8.f;

	// ==================== COMBAT STATS ====================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Combat")
	float MaxHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Combat")
	float Health = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Combat")
	float KineticShields = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Combat")
	float MaxKineticShields = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Combat")
	float EnergyShields = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Combat")
	float MaxEnergyShields = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Combat|Trauma")
	float FreshTrauma = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Combat|Trauma")
	float OldTrauma = 0.f;

	// ==================== TURN STATS ====================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Turn")
	float ActionPoints = 2.f;

	/** Max walk distance per turn, in feet. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats|Turn")
	float MaxWalkDistance = 30.f;

	// ==================== ABILITIES ====================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;

	// ==================== TAGS ====================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS")
	FGameplayTagContainer StartupTags;

	// ==================== TUNING ====================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tuning")
	TMap<FName, FScalableFloat> Scalars;
};
