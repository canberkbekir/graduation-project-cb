#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/RAbilityDisplayRow.h"
#include "Events/WorldViewEvents.h"
#include "UI/Models/RActionBarModels.h"
#include "RAbilityDisplayLibrary.generated.h"

class UAbilitySystemComponent;
class UDataTable;

UCLASS()
class RIZZGAME_API URAbilityDisplayLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Find a display row by exact tag match. Returns nullptr if not found. (C++ only) */
	static const FAbilityDisplayRow* FindDisplayRowForTag(
		const UDataTable* DisplayTable, const FGameplayTag& AbilityTag);

	/** BP-friendly version: copies the row into OutRow, returns true if found. */
	UFUNCTION(BlueprintCallable, Category="Rizz|Abilities")
	static bool FindDisplayRowForTag_BP(
		const UDataTable* DisplayTable, const FGameplayTag& AbilityTag,
		FAbilityDisplayRow& OutRow);

	/**
	 * Build view-model array from the ASC's granted abilities.
	 * Filters by bShowInActionBar and WorldView, sorts by SortOrder, resolves icons via LoadSynchronous.
	 */
	UFUNCTION(BlueprintCallable, Category="Rizz|Abilities")
	static TArray<FActionSlotViewModel> BuildActionBarViewModels(
		UAbilitySystemComponent* ASC, const UDataTable* DisplayTable,
		EWorldView WorldView = EWorldView::Physical);

#if WITH_EDITOR
	/**
	 * [EDITOR ONLY] Scans all registered GameplayTags under "Ability" and adds
	 * missing rows to the DataTable for each leaf tag (tags with no children).
	 *
	 * How to use:
	 *  1. Create an Editor Utility Widget (right-click Content Browser → Editor Utilities → Editor Utility Widget)
	 *  2. Add a Button, bind its OnClicked to call this function
	 *  3. Pass your DT_AbilityDisplay DataTable as the parameter
	 *  4. Click the button — new rows appear for any leaf Ability.* tags not already in the table
	 *  5. Review and fill in Icon, Description, AP cost etc. for the generated rows
	 *
	 * Generated rows get:
	 *  - RowName = tag string (e.g. "Ability.Attack.Melee.HeavyPhysicalMelee")
	 *  - AbilityTag = the matching GameplayTag
	 *  - DisplayName = human-readable name derived from the leaf (e.g. "Heavy Physical Melee")
	 *  - All other fields = defaults (white tint, AP cost 1, SortOrder 0, bShowInActionBar true)
	 *
	 * @param DisplayTable  The DataTable (row type FAbilityDisplayRow) to populate.
	 * @return Number of new rows added.
	 */
	UFUNCTION(BlueprintCallable, Category="Rizz|Abilities|Editor", meta=(DevelopmentOnly))
	static int32 AutoPopulateFromGameplayTags(UDataTable* DisplayTable);
#endif
};
