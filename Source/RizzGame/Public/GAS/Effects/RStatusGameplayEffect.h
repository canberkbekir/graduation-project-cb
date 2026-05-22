// RStatusGameplayEffect.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GAS/RCombatTypes.h"
#include "Subsystems/DiceSubsystem.h"
#include "Data/RStatusEffectDisplayRow.h"
#include "RStatusGameplayEffect.generated.h"

/** How a turn-based status effect deals damage each tick. */
UENUM(BlueprintType)
enum class ERDamageTickType : uint8
{
	None        UMETA(DisplayName = "No Damage"),
	Fixed       UMETA(DisplayName = "Fixed Amount"),
	Dice        UMETA(DisplayName = "Dice Roll"),
	StatBased   UMETA(DisplayName = "Stat-Based"),
};

/** Determines where the Difficulty Class comes from at cast time. */
UENUM(BlueprintType)
enum class EDCSource : uint8
{
	Fixed,      // Use the AbilityDC integer directly
	Physique,   // DC = caster's raw Physique value
	Tech,       // DC = caster's raw Tech value
	Mind,       // DC = caster's raw Mind value
	Insight,    // DC = caster's raw Insight value
	Finesse,    // DC = caster's raw Finesse value
};

/**
 * Base class for status effects (poison, bleed, destabilized, etc.).
 * Carries resistance type, DC source, and save requirement so each GE asset
 * self-describes how it is resisted — no code changes needed per new condition.
 *
 * Designer workflow:
 *   1. Create GE Blueprint → parent to URStatusGameplayEffect.
 *   2. Set DCSource (Fixed or a caster stat), ResistanceType, AbilityDC (if Fixed).
 *   3. Set Duration, Period, and Modifiers as normal in the GE asset.
 *   4. Assign to the ability's secondary effect class.
 */
UCLASS(BlueprintType, Blueprintable)
class RIZZGAME_API URStatusGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	/** If false the effect always applies — no saving throw is rolled. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Status|Saving Throw")
	bool bRequiresSavingThrow = true;

	/** Which resistance stat the target uses when rolling the save. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Status|Saving Throw",
		meta=(EditCondition="bRequiresSavingThrow"))
	EResistanceType ResistanceType = EResistanceType::Physical;

	/** Where the DC comes from. Select a stat to use the caster's raw attribute value. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Status|Saving Throw",
		meta=(EditCondition="bRequiresSavingThrow"))
	EDCSource DCSource = EDCSource::Fixed;

	/** Used only when DCSource = Fixed. Ignored otherwise. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Status|Saving Throw",
		meta=(EditCondition="DCSource==EDCSource::Fixed", ClampMin="1"))
	int32 AbilityDC = 10;

	// ─── Turn-Based Tick Configuration ────────────────────────────────────────

	/** How this effect deals damage each tick. None = modifier-only debuff. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Status|Tick Damage")
	ERDamageTickType DamageTickType = ERDamageTickType::None;

	/** Which AdvanceTurn phase triggers the tick (own turn start or every turn). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Status|Tick Damage",
		meta=(EditCondition="DamageTickType != ERDamageTickType::None"))
	EREffectTickMode TickMode = EREffectTickMode::OnOwnTurnOnly;

	/**
	 * Instant GE applied each tick to deal damage.
	 * Point to GE_PhysicalDamage or GE_NetworkDamage.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Status|Tick Damage",
		meta=(EditCondition="DamageTickType != ERDamageTickType::None"))
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** SetByCaller tag the damage GE's execution calculation reads (e.g. Data.Damage.Physical). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Status|Tick Damage",
		meta=(EditCondition="DamageTickType != ERDamageTickType::None"))
	FGameplayTag DamageSetByCallerTag;

	/** Fixed: flat damage per tick. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Status|Tick Damage",
		meta=(EditCondition="DamageTickType == ERDamageTickType::Fixed"))
	float FixedDamageAmount = 0.f;

	/** Dice: expression rolled at tick time (e.g. Count=1, Die=D4 for 1d4). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Status|Tick Damage",
		meta=(EditCondition="DamageTickType == ERDamageTickType::Dice"))
	FDiceExpression TickDiceRoll;

	/**
	 * StatBased: attribute read from the target at tick time; result * StatCoefficient = damage.
	 * Use tags: "Stat.Tech", "Stat.Physique", "Stat.Insight", "Stat.Mind", "Stat.Finesse".
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Status|Tick Damage",
		meta=(EditCondition="DamageTickType == ERDamageTickType::StatBased"))
	FGameplayTag StatAttributeTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Status|Tick Damage",
		meta=(EditCondition="DamageTickType == ERDamageTickType::StatBased"))
	float StatCoefficient = 0.5f;

	/**
	 * Reads the saving throw configuration from a GE class.
	 * Returns false if EffectClass is not a URStatusGameplayEffect.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Status",
		meta=(DisplayName="Get Status Effect Save Info"))
	static bool GetSaveInfo(TSubclassOf<UGameplayEffect> EffectClass,
		bool& bOutRequiresSave, EResistanceType& OutResistanceType, int32& OutDC);

	/**
	 * Computes the actual DC at cast time.
	 * If DCSource = Fixed, returns AbilityDC.
	 * Otherwise reads the matching raw stat from Source's CoreAttributeSet.
	 * Returns 10 as fallback if Source or its ASC is missing.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Status",
		meta=(DisplayName="Compute Ability DC"))
	static int32 ComputeDC(TSubclassOf<UGameplayEffect> EffectClass, AActor* Source);
};
