#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/RStatusEffectDisplayRow.h"
#include "UI/Models/RCombatUIModels.h"
#include "RStatusEffectLibrary.generated.h"

class UAbilitySystemComponent;
class UDataTable;

UCLASS()
class RIZZGAME_API URStatusEffectLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Query active gameplay effects on the ASC and build icon view models
	 * by matching effect asset tags against the DataTable rows.
	 */
	UFUNCTION(BlueprintCallable, Category="Rizz|StatusEffects")
	static TArray<FEffectIconViewModel> GetActiveEffectIcons(
		UAbilitySystemComponent* ASC,
		const UDataTable* DisplayTable);

	/**
	 * Tick all turn-based effects whose TickMode matches the given filter.
	 * Decrements TurnsRemaining, re-applies the GE's modifiers as an instant effect,
	 * and auto-removes effects that hit 0 turns.
	 */
	UFUNCTION(BlueprintCallable, Category="Rizz|StatusEffects")
	static void TickTurnEffects(
		UAbilitySystemComponent* ASC,
		const UDataTable* DisplayTable,
		EREffectTickMode TickMode);

#if WITH_EDITOR
	/**
	 * [EDITOR ONLY] Scans all URStatusGameplayEffect subclasses, reads their
	 * granted tags and auto-adds missing rows to the DataTable.
	 *
	 * Generated rows get:
	 *  - EffectTag           = first granted tag on the GE
	 *  - DisplayName         = human-readable name from the leaf tag
	 *  - GameplayEffectClass = the GE class itself
	 *  - All other fields    = sensible defaults (white tint, 3 turns, bShowStacks true)
	 *
	 * @param DisplayTable  DataTable with row type FStatusEffectDisplayRow.
	 * @return Number of new rows added.
	 */
	UFUNCTION(BlueprintCallable, Category="Rizz|StatusEffects|Editor", meta=(DevelopmentOnly))
	static int32 AutoPopulateEffectDisplayTable(UDataTable* DisplayTable);
#endif
};
