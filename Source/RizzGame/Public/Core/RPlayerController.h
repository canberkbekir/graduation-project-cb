#pragma once

#include "CoreMinimal.h"
#include "Core/RPlayerControllerBase.h"
#include "RPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class URClickResolver;
class URActionBarWidget;
class ARCharacterBase;

/**
 * Combat-capable player controller. Extends the base controller with:
 *   - CombatMappingContext (swapped in/out with ExplorationMappingContext)
 *   - URClickResolver (context-sensitive click routing)
 *   - ActionBarWidget integration (ability selection)
 *
 * Setup in Blueprint:
 *   1. Assign BaseMappingContext, ExplorationMappingContext (inherited).
 *   2. Assign CombatMappingContext, ClickAction, SecondaryClickAction.
 *   3. Call SetActionBar() from the HUD after creating the action bar widget.
 */
UCLASS()
class RIZZGAME_API ARPlayerController : public ARPlayerControllerBase
{
	GENERATED_BODY()

public:
	ARPlayerController();

	UFUNCTION(BlueprintCallable, Category="Input")
	void SetActionBar(URActionBarWidget* InActionBar);

	URActionBarWidget* GetActionBarWidget() const { return ActionBarWidget.Get(); }

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Active only during combat. Swapped in when FCombatStarted fires, out on FCombatEnded. */
	UPROPERTY(EditDefaultsOnly, Category="R|Input")
	TObjectPtr<UInputMappingContext> CombatMappingContext;

	/** The primary world-interaction click (ability targeting, character selection, movement). */
	UPROPERTY(EditDefaultsOnly, Category="R|Input|Combat")
	TObjectPtr<UInputAction> ClickAction;

	/** Cancels the current ability selection or deselects the active character. */
	UPROPERTY(EditDefaultsOnly, Category="R|Input|Combat")
	TObjectPtr<UInputAction> SecondaryClickAction;

	/** Channel used for world traces (ground, interactables, static meshes). */
	UPROPERTY(EditDefaultsOnly, Category="R|Trace")
	TEnumAsByte<ECollisionChannel> WorldTraceChannel = ECC_Visibility;

	/** Toggle hover/click diagnostic logs. Off by default — enable in Blueprint defaults when debugging input. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="R|Debug")
	bool bEnableHoverLogs = false;

	/** Object types queried to detect characters under the cursor.
	 *  Checked before the world trace — any hit that casts to ARCharacterBase wins.
	 *  Add custom channels here if your characters use a non-Pawn object type. */
	UPROPERTY(EditDefaultsOnly, Category="R|Trace")
	TArray<TEnumAsByte<EObjectTypeQuery>> CharacterObjectTypes;

protected:
	// --- Ability targeting hover (fires when ability is selected and cursor is over a character) ---

	/** Called when the cursor enters a valid ability target while an ability is selected. */
	UFUNCTION(BlueprintImplementableEvent, Category="R|Ability|Targeting")
	void OnAbilityTargetHoverBegin(AActor* Target);

	/** Called when the cursor leaves a valid ability target or the ability is deselected. */
	UFUNCTION(BlueprintImplementableEvent, Category="R|Ability|Targeting")
	void OnAbilityTargetHoverEnd(AActor* PreviousTarget);

private:
	// --- Input handlers ---
	void OnClick();
	void OnSecondaryClick();

	// --- Per-frame hover ---
	void UpdateHover();

	// --- Combat context swap (driven by EventBus) ---
	void OnCombatStarted(const struct FCombatStarted& Event);
	void OnCombatEnded(const struct FCombatEnded& Event);

	// --- State queries ---
	bool IsInCombat() const;
	bool IsPlayerTurn() const;

	// --- Members ---
	UPROPERTY()
	TObjectPtr<URClickResolver> ClickResolver;

	TWeakObjectPtr<AActor> HoveredActor;
	TWeakObjectPtr<AActor> AbilityHoveredTarget;
	TWeakObjectPtr<URActionBarWidget> ActionBarWidget;
	TWeakObjectPtr<ARCharacterBase> HoveredCharacter;
	TWeakObjectPtr<ARCharacterBase> PendingHideCharacter;
	FTimerHandle CharacterHideTimerHandle;
};
