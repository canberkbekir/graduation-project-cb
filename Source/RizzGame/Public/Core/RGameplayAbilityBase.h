// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AITypes.h"
#include "Abilities/GameplayAbility.h"
#include "Navigation/PathFollowingComponent.h"
#include "Events/AbilityEvents.h"
#include "Events/CheckEvents.h"
#include "Events/WorldViewEvents.h"
#include "GAS/RCombatTypes.h"
#include "GAS/Effects/RStatusGameplayEffect.h"
#include "RGameplayAbilityBase.generated.h"

class AAIController;

UENUM(BlueprintType)
enum class ETargetingType : uint8
{
	SelfCast    UMETA(DisplayName="Self Cast"),
	SingleActor UMETA(DisplayName="Single Actor"),
	AOE         UMETA(DisplayName="Area of Effect"),
};

USTRUCT(BlueprintType)
struct FAbilityIndicatorConfig
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	ETargetingType TargetingType = ETargetingType::SelfCast;

	UPROPERTY(BlueprintReadWrite)
	float Range = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float AOERadius = 0.f;
};

/**
 *
 */
UCLASS()
class RIZZGAME_API URGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	URGameplayAbilityBase();

	/** Backward-compat: true for any type that requires a target click (non-SelfCast). */
	UFUNCTION(BlueprintPure, Category="Ability")
	bool IsTargetRequired() const { return TargetingType != ETargetingType::SelfCast; }

	UFUNCTION(BlueprintPure, Category="Ability|WorldView")
	EWorldView GetWorldViewAvailability() const { return WorldViewAvailability; }

	// --- Montage Getters ---

	UFUNCTION(BlueprintPure, Category="Ability|Animation")
	UAnimMontage* GetAbilitySelectMontage() const { return AbilitySelectMontage; }

	UFUNCTION(BlueprintPure, Category="Ability|Animation")
	UAnimMontage* GetAfterAbilitySelectMontage() const { return AfterAbilitySelectMontage; }

	UFUNCTION(BlueprintPure, Category="Ability|Animation")
	UAnimMontage* GetAbilityExecuteMontage() const { return AbilityExecuteMontage; }

	// --- Indicator ---

	/** Returns config data used by the indicator system (range circle radius, AOE radius, type). Override in BP for custom indicators. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ability|Targeting")
	FAbilityIndicatorConfig GetIndicatorConfig() const;
	virtual FAbilityIndicatorConfig GetIndicatorConfig_Implementation() const;

	/** Draws debug circles for range and AOE each frame while the ability is selected. Call from widget NativeTick. */
	UFUNCTION(BlueprintCallable, Category="Ability|Debug")
	void DrawDebugIndicators(const FVector& SelfLocation, const FVector& CursorLocation) const;

	/** Called when the ability is selected from the action bar. Override in BP_GameplayAbilityBase to show indicators. */
	UFUNCTION(BlueprintNativeEvent, Category="Ability|Indicator")
	void ShowIndicator(AActor* Avatar);
	virtual void ShowIndicator_Implementation(AActor* Avatar) {}

	/** Called when the ability is deselected. Override in BP_GameplayAbilityBase to hide/destroy indicators. */
	UFUNCTION(BlueprintNativeEvent, Category="Ability|Indicator")
	void HideIndicator();
	virtual void HideIndicator_Implementation() {}

	/** Called each frame while the ability is selected. Override in BP_GameplayAbilityBase to move AOE previews to the cursor. */
	UFUNCTION(BlueprintNativeEvent, Category="Ability|Indicator")
	void TickIndicator(const FVector& CursorWorldPos);
	virtual void TickIndicator_Implementation(const FVector& CursorWorldPos) {}

	// --- Selection Lifecycle (called by ActionBar, not GAS activation) ---

	/** Called when the ability is selected from the action bar. Starts AbilitySelectMontage -> AfterAbilitySelectMontage. */
	UFUNCTION(BlueprintCallable, Category="Ability|Selection")
	virtual void OnAbilitySelectedFromBar();

	/**
	 * Called when the ability is deselected from the action bar.
	 * Stops selection montages. When bIsCancelled is true (right-click, new ability selected),
	 * also aborts any in-flight approach walk and ends the ability.
	 * When false (ability was just executed), the approach walk continues to completion.
	 */
	UFUNCTION(BlueprintCallable, Category="Ability|Selection")
	virtual void OnAbilityDeselectedFromBar(bool bIsCancelled = false);

	// --- Navigation Helpers ---

	/** Returns the navmesh path length from Start to End in UE units, or -1 if no valid path exists. */
	UFUNCTION(BlueprintCallable, Category="Ability|Targeting", meta=(WorldContext="WorldContextObject"))
	static float GetNavPathLength(UObject* WorldContextObject, const FVector& Start, const FVector& End);

	/** Returns the navmesh point the character should walk to before executing this ability.
	 *  Default: projects (TargetLocation + (SelfLocation - TargetLocation).Normalize() * Range * RangeApproachMargin)
	 *  onto the navmesh, with an 8-direction sweep as fallback.
	 *  Override per ability for melee, flanking, AOE, or custom approach behavior.
	 *  Returns TargetLocation unchanged if no valid navmesh point can be projected — caller treats this as unreachable. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ability|Targeting")
	FVector ComputeApproachLocation(const FVector& SelfLocation, const FVector& TargetLocation) const;
	virtual FVector ComputeApproachLocation_Implementation(const FVector& SelfLocation, const FVector& TargetLocation) const;

	// --- GAS Overrides ---

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr)
	const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData)
	override;

	virtual void ApplyCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo)
	const override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

protected:
	// --- Blueprint Events ---

	UFUNCTION(BlueprintImplementableEvent, Category="Ability|Selection")
	void BP_OnAbilitySelectedFromBar();

	UFUNCTION(BlueprintImplementableEvent, Category="Ability|Selection")
	void BP_OnAbilityDeselectedFromBar();

	UFUNCTION(BlueprintCallable)
	virtual bool GetHitResultUnderCursor(FHitResult& OutHitResult) const;
	FORCEINLINE virtual bool IsCharacterInTurn(const ACharacter* Character) const;

	virtual float GetEffectiveRange() const { return Range; }

	/** Stops any playing selection montages with a 0.25s blend-out. Called before deselect or execution. */
	void StopSelectionMontages();

	/**
	 * Rolls the dice expression, publishes FAbilityResolved, and returns the result.
	 * Use this when you need to apply effects in a custom way (projectile, multi-target, etc.).
	 */
	UFUNCTION(BlueprintCallable, Category="Ability|Resolution")
	FDiceRollResult RollAndBroadcast(
		AActor* Source,
		AActor* Target,
		const FDiceExpression& Expression,
		FGameplayTag MagnitudeTag,
		EDiceRollContext RollContext);

	/**
	 * Rolls the dice expression, applies a SetByCaller GameplayEffect to Target, and broadcasts FAbilityResolved.
	 * The one-liner for standard damage/heal abilities. Returns the dice result for any extra use.
	 * If EffectClass is null, falls back to RollAndBroadcast only (no GE applied).
	 * Multiplier scales the rolled total before applying (2.0 for critical hits, 0.5 for half-effect saves).
	 * The broadcast FAbilityResolved carries the already-multiplied total so logs see the real number.
	 */
	UFUNCTION(BlueprintCallable, Category="Ability|Resolution")
	FDiceRollResult ApplyEffectWithRoll(
		AActor* Source,
		AActor* Target,
		TSubclassOf<UGameplayEffect> EffectClass,
		const FDiceExpression& Expression,
		FGameplayTag MagnitudeTag,
		EDiceRollContext RollContext,
		float Multiplier = 1.f,
		int32 DurationTurns = 0);

	/**
	 * Rolls 1d20 + AttackBonus vs the target's Dodge (10 + FinesseMod).
	 * Natural 20 → CriticalHit regardless of the target's Dodge.
	 * Broadcasts FCheckResolved (tag: Check.Hit) so the log system picks it up.
	 */
	UFUNCTION(BlueprintCallable, Category="Ability|Combat")
	FHitCheckResult PerformHitCheck(
		AActor* Source,
		AActor* Target,
		int32 AttackBonus,
		EDiceRollContext Ctx);

	/**
	 * Rolls 1d20 + target's resistance stat modifier vs DC.
	 * Natural 20 on the save → CriticalSuccess (negate effect).
	 * Roll >= DC → Success (half effect). Roll < DC → Fail (full effect).
	 * Broadcasts FCheckResolved (tag: Check.Save.<Type>) so the log system picks it up.
	 */
	UFUNCTION(BlueprintCallable, Category="Ability|Combat")
	FSaveCheckResult PerformSavingThrow(
		AActor* Source,
		AActor* Target,
		EResistanceType Type,
		int32 DC,
		EDiceRollContext Ctx);

	/**
	 * Convenience wrapper: reads ResistanceType, DCSource, and DC directly from a URStatusGameplayEffect CDO.
	 * Computes DC from Source's stat if DCSource != Fixed.
	 * Falls back to Fail outcome if EffectClass is not a URStatusGameplayEffect.
	 */
	UFUNCTION(BlueprintCallable, Category="Ability|Combat")
	FSaveCheckResult PerformSavingThrowForEffect(
		AActor* Source,
		AActor* Target,
		TSubclassOf<UGameplayEffect> EffectClass,
		EDiceRollContext Ctx);

	/**
	 * Collects all ARCharacterBase actors within Radius of Center.
	 * bIncludeAllies / bIncludeEnemies filter by team relative to the ability's avatar actor.
	 * Self is always excluded.
	 */
	UFUNCTION(BlueprintCallable, Category="Ability|Targeting")
	TArray<AActor*> CollectActorsInRadius(
		const FVector& Center,
		float Radius,
		bool bIncludeAllies,
		bool bIncludeEnemies);

	/** Shared pre-activation setup: stops selection montages and fires the grandparent ActivateAbility.
	 *  Does NOT commit AP cost — that is now done inside CommitAbilityExecution_Implementation.
	 *  Returns false (and ends the ability) if grandparent activation fails.
	 *  Call this from ActivateAbility overrides in subclasses that manage their own execution flow
	 *  (e.g. URGA_Walk, URGA_Interact) instead of calling Super::ActivateAbility(). */
	bool ActivationSetup(
		const FGameplayAbilitySpecHandle& Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo& ActivationInfo,
		const FGameplayEventData* TriggerEventData);

	/** Commits AP cost then executes the ability.
	 *  BlueprintNativeEvent — designers override this in Blueprint to implement ability logic
	 *  (play AbilityExecuteMontage, apply GameplayEffects, spawn projectiles, etc.).
	 *  Default C++ impl commits AP cost; if CommitAbility fails, cancels via EndAbility.
	 *  Call EndAbility() when the ability finishes its work. */
	UFUNCTION(BlueprintNativeEvent, Category="Ability")
	void CommitAbilityExecution();
	virtual void CommitAbilityExecution_Implementation();

	// --- Combat Resolution Properties ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Combat",
		meta=(ToolTip="Bonus added to the 1d20 attack roll used by PerformHitCheck. Represents weapon accuracy, class training, etc."))
	int32 AttackMod = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Combat",
		meta=(ToolTip="When true, this ability's secondary effect requires the target to make a saving throw before it is applied."))
	bool bRequiresSavingThrow = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Combat",
		meta=(EditCondition="bRequiresSavingThrow",
		      ToolTip="Which resistance stat the target uses when making a saving throw against this ability's secondary effects."))
	EResistanceType ResistanceType = EResistanceType::Physical;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Combat",
		meta=(EditCondition="bRequiresSavingThrow", ClampMin="1",
		      ToolTip="Difficulty Class for saving throws against this ability's secondary effects."))
	int32 AbilityDC = 10;

	// --- Animation Properties ---

	UPROPERTY(EditDefaultsOnly, Category="Ability|Animation",
		meta=(ToolTip="Plays once on successful execution (e.g. attack swing, spell cast). Subclasses trigger this in CommitAbilityExecution."))
	TObjectPtr<UAnimMontage> AbilityExecuteMontage;

	UPROPERTY(EditDefaultsOnly, Category="Ability|Animation",
		meta=(ToolTip="Plays once when the ability is selected from the action bar (intro/draw animation). When it ends, AfterAbilitySelectMontage starts. Leave empty to skip directly to AfterAbilitySelectMontage."))
	TObjectPtr<UAnimMontage> AbilitySelectMontage;

	UPROPERTY(EditDefaultsOnly, Category="Ability|Animation",
		meta=(ToolTip="Idle/hold animation played while waiting for a target click. Looping is configured by the animator in the montage section settings — code does not loop. Stops with a 0.25s blend-out before deselect or execute."))
	TObjectPtr<UAnimMontage> AfterAbilitySelectMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability",
		meta=(ToolTip="Optional GameplayEffect applied to the target(s). Subclasses apply this in CommitAbilityExecution."))
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Targeting",
		meta=(ToolTip="Determines how the ability selects its target. SelfCast: no cursor interaction, executes immediately. SingleActor: requires clicking an actor. AOE: requires clicking a location, affects AOERadius area."))
	ETargetingType TargetingType = ETargetingType::SingleActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability",
		meta=(ClampMin="0.0",
		ToolTip="Ability range in UE units (cm). If the navmesh path length to the target exceeds this value and bAutoApproachToRange is true, the character walks toward the target first. 0 = no range restriction."))
	float Range = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Targeting",
		meta=(ClampMin="0.0",
		ToolTip="Radius of the AOE effect in UE units (cm). Only used when TargetingType is AOE.",
		EditCondition="TargetingType==ETargetingType::AOE"))
	float AOERadius = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Targeting",
		meta=(ToolTip="If true and the target is out of range, the character walks (within the remaining turn WalkDistance budget) to enter range, then auto-executes. If false, out-of-range target clicks silently cancel the ability."))
	bool bAutoApproachToRange = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Targeting",
		meta=(ClampMin="0.0", ClampMax="1.0",
		ToolTip="Fraction of Range used as the approach distance (0.9 = stop at 90% of Range, leaving a small buffer). MoveTo AcceptanceRadius is derived as Range * (1 - RangeApproachMargin). Default 0.9."))
	float RangeApproachMargin = 0.9f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|WorldView",
		meta=(ToolTip="Which world view this ability is available in."))
	EWorldView WorldViewAvailability = EWorldView::Physical;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Ability|Debug",
		meta=(ToolTip="When selected, draws a yellow range circle around the character and (AOE only) a red AOE circle at the cursor each frame."))
	bool bShowDebugIndicators = true;

	FHitResult HitResult;

private:
	// --- Combat helpers ---

	/** floor((RawStat - 10) / 2). Same formula as D&D: stat 8→-1, 10→0, 12→+1, 14→+2. */
	static int32 StatModifier(float RawStat);

	/** Returns the target's Dodge value: 10 + StatModifier(Finesse). */
	int32 GetDodgeValue(AActor* Target) const;

	/** Returns the raw attribute value for the resistance stat that corresponds to Type. */
	float GetResistanceStat(AActor* Target, EResistanceType Type) const;

	/** Publishes FCheckResolved to the EventBus. Called after every hit/save roll. */
	void BroadcastCheckResolved(AActor* Src, AActor* Tgt, FGameplayTag Tag,
		const FDiceExpression& Expr, const FDiceRollResult& Roll,
		int32 TargetValue, bool bSuccess, bool bCrit);

	void OnSelectMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** Shared range-check and approach-walk logic for SingleActor and AOE after target point is resolved. */
	void HandleTargetedActivation(
		const FGameplayAbilitySpecHandle& Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo& ActivationInfo,
		const FVector& TargetPoint);

	/** Starts the approach walk. Walks PathLength if within budget, otherwise walks WalkBudget distance
	 *  and decrements WalkDistance to 0. AP is NOT committed here — only in CommitAbilityExecution. */
	void StartApproachWalk(
		const FGameplayAbilitySpecHandle& Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo& ActivationInfo,
		AAIController* AICon,
		UAbilitySystemComponent* Asc,
		const FVector& SelfLocation,
		const FVector& ApproachGoal,
		float PathLength,
		float WalkBudget);

	/** Walks the navpath from Start toward End and returns the point exactly BudgetDistance along it. */
	static FVector ComputeWaypointAlongPath(const UObject* WorldContext, const FVector& Start, const FVector& End, float BudgetDistance);

	// --- Pending approach-walk state ---
	bool bHasPendingApproachWalk = false;
	FAIRequestID PendingApproachMoveID;
	TWeakObjectPtr<UPathFollowingComponent> PendingApproachPathComp;
	FTimerHandle PendingApproachNextTickTimer;

	void OnApproachWalkFinished(FAIRequestID RequestID, const FPathFollowingResult& Result);
	void ExecuteAbilityAfterTick();
};
