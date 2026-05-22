#include "GAS/RAbilityDisplayLibrary.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Core/RGameplayAbilityBase.h"
#include "Engine/DataTable.h"

#if WITH_EDITOR
#include "GameplayTagsManager.h"
#endif

const FAbilityDisplayRow* URAbilityDisplayLibrary::FindDisplayRowForTag(
	const UDataTable* DisplayTable, const FGameplayTag& AbilityTag)
{
	if (!DisplayTable || !AbilityTag.IsValid())
	{
		return nullptr;
	}

	static const FString ContextString(TEXT("FindDisplayRowForTag"));
	TArray<FAbilityDisplayRow*> Rows;
	DisplayTable->GetAllRows(ContextString, Rows);

	for (const FAbilityDisplayRow* Row : Rows)
	{
		if (Row && Row->AbilityTag.MatchesTagExact(AbilityTag))
		{
			return Row;
		}
	}

	return nullptr;
}

bool URAbilityDisplayLibrary::FindDisplayRowForTag_BP(
	const UDataTable* DisplayTable, const FGameplayTag& AbilityTag,
	FAbilityDisplayRow& OutRow)
{
	const FAbilityDisplayRow* Row = FindDisplayRowForTag(DisplayTable, AbilityTag);
	if (Row)
	{
		OutRow = *Row;
		return true;
	}
	return false;
}

TArray<FActionSlotViewModel> URAbilityDisplayLibrary::BuildActionBarViewModels(
	UAbilitySystemComponent* ASC, const UDataTable* DisplayTable, EWorldView WorldView)
{
	TArray<FActionSlotViewModel> Result;

	if (!ASC || !DisplayTable)
	{
		return Result;
	}

	const TArray<FGameplayAbilitySpec>& Specs = ASC->GetActivatableAbilities();

	UE_LOG(LogTemp, Warning, TEXT("BuildActionBarViewModels: ASC Owner='%s', ActivatableAbilities=%d"),
		*GetNameSafe(ASC->GetOwner()), Specs.Num());

	for (const FGameplayAbilitySpec& Spec : Specs)
	{
		const UGameplayAbility* AbilityCDO = Spec.Ability;
		if (!AbilityCDO)
		{
			continue;
		}

		// Get ability tags from the CDO
		const FGameplayTagContainer& AbilityTags = AbilityCDO->GetAssetTags();

		UE_LOG(LogTemp, Warning, TEXT("  Ability '%s': AssetTags=%d (%s)"),
			*GetNameSafe(AbilityCDO->GetClass()),
			AbilityTags.Num(),
			*AbilityTags.ToStringSimple());

		// Find a matching display row
		const FAbilityDisplayRow* FoundRow = nullptr;
		for (const FGameplayTag& Tag : AbilityTags)
		{
			FoundRow = FindDisplayRowForTag(DisplayTable, Tag);
			if (FoundRow)
			{
				break;
			}
		}

		if (!FoundRow)
		{
			UE_LOG(LogTemp, Warning, TEXT("    -> SKIPPED: no matching display row found for any tag."));
			continue;
		}
		if (!FoundRow->bShowInActionBar)
		{
			UE_LOG(LogTemp, Warning, TEXT("    -> SKIPPED: bShowInActionBar is false."));
			continue;
		}

		FActionSlotViewModel VM;
		VM.AbilityTag = FoundRow->AbilityTag;
		VM.DisplayName = FoundRow->DisplayName;
		VM.Description = FoundRow->Description;
		VM.Icon = FoundRow->Icon.LoadSynchronous();
		VM.TintColor = FoundRow->TintColor;
		VM.ActionPointCost = FoundRow->ActionPointCost;

		// WorldView filter and bRequiresTarget from the ability CDO
		if (const URGameplayAbilityBase* RAbilityCDO = Cast<URGameplayAbilityBase>(AbilityCDO))
		{
			if (RAbilityCDO->GetWorldViewAvailability() != WorldView)
			{
				UE_LOG(LogTemp, Warning, TEXT("    -> SKIPPED: WorldViewAvailability does not match current WorldView."));
				continue;
			}
			VM.bRequiresTarget = RAbilityCDO->IsTargetRequired();
		}

		// Determine slot state
		const FGameplayTagContainer TagContainer(FoundRow->AbilityTag);
		if (ASC->AreAbilityTagsBlocked(AbilityTags))
		{
			VM.SlotState = EActionSlotState::Blocked;
		}
		else
		{
			VM.SlotState = EActionSlotState::Available;
		}

		Result.Add(VM);
	}

	// Sort by SortOrder (look up the row again for sort key)
	Result.Sort([&DisplayTable](const FActionSlotViewModel& A, const FActionSlotViewModel& B)
	{
		const FAbilityDisplayRow* RowA = FindDisplayRowForTag(DisplayTable, A.AbilityTag);
		const FAbilityDisplayRow* RowB = FindDisplayRowForTag(DisplayTable, B.AbilityTag);
		const int32 OrderA = RowA ? RowA->SortOrder : 0;
		const int32 OrderB = RowB ? RowB->SortOrder : 0;
		return OrderA < OrderB;
	});

	// Assign slot indices
	for (int32 i = 0; i < Result.Num(); ++i)
	{
		Result[i].SlotIndex = i;
	}

	return Result;
}

// ────────────── Editor-Only: Auto-Populate DataTable ──────────────

#if WITH_EDITOR

/**
 * Convert a PascalCase or camelCase tag leaf into a human-readable name.
 * "HeavyPhysicalMelee" → "Heavy Physical Melee"
 */
static FString PascalToDisplayName(const FString& PascalString)
{
	if (PascalString.IsEmpty())
	{
		return PascalString;
	}

	FString Result;
	Result.Reserve(PascalString.Len() + 8);

	for (int32 i = 0; i < PascalString.Len(); ++i)
	{
		const TCHAR Ch = PascalString[i];

		// Insert space before an uppercase letter that follows a lowercase letter
		if (i > 0 && FChar::IsUpper(Ch) && FChar::IsLower(PascalString[i - 1]))
		{
			Result.AppendChar(TEXT(' '));
		}

		Result.AppendChar(Ch);
	}

	return Result;
}

/**
 * Recursively collect leaf tags (tags with no children) under a parent tag.
 */
static void CollectLeafTags(
	const UGameplayTagsManager& Manager,
	const FGameplayTag& ParentTag,
	TArray<FGameplayTag>& OutLeafTags)
{
	FGameplayTagContainer ChildTags = Manager.RequestGameplayTagChildren(ParentTag);

	if (ChildTags.Num() == 0)
	{
		// This is a leaf — no children
		OutLeafTags.Add(ParentTag);
		return;
	}

	// Recurse into children
	for (const FGameplayTag& ChildTag : ChildTags)
	{
		CollectLeafTags(Manager, ChildTag, OutLeafTags);
	}
}

int32 URAbilityDisplayLibrary::AutoPopulateFromGameplayTags(UDataTable* DisplayTable)
{
	if (!DisplayTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoPopulateFromGameplayTags: DisplayTable is null."));
		return 0;
	}

	// Verify the DataTable's row struct
	if (DisplayTable->GetRowStruct() != FAbilityDisplayRow::StaticStruct())
	{
		UE_LOG(LogTemp, Error,
			TEXT("AutoPopulateFromGameplayTags: DataTable '%s' row type is not FAbilityDisplayRow."),
			*DisplayTable->GetName());
		return 0;
	}

	const UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
	const FGameplayTag AbilityRoot = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability")), false);

	if (!AbilityRoot.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoPopulateFromGameplayTags: 'Ability' root tag not found."));
		return 0;
	}

	// Collect all leaf tags under Ability.*
	TArray<FGameplayTag> LeafTags;
	CollectLeafTags(TagManager, AbilityRoot, LeafTags);

	// Gather existing tags in the table to skip duplicates
	TSet<FGameplayTag> ExistingTags;
	{
		static const FString Context(TEXT("AutoPopulate_ExistingCheck"));
		TArray<FAbilityDisplayRow*> ExistingRows;
		DisplayTable->GetAllRows(Context, ExistingRows);
		for (const FAbilityDisplayRow* Row : ExistingRows)
		{
			if (Row)
			{
				ExistingTags.Add(Row->AbilityTag);
			}
		}
	}

	int32 AddedCount = 0;

	for (const FGameplayTag& Tag : LeafTags)
	{
		if (ExistingTags.Contains(Tag))
		{
			continue;
		}

		// Extract the leaf portion of the tag for display name
		// e.g. "Ability.Attack.Melee.HeavyPhysicalMelee" → "HeavyPhysicalMelee"
		const FString TagString = Tag.ToString();
		FString LeafName;
		TagString.Split(TEXT("."), nullptr, &LeafName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		if (LeafName.IsEmpty())
		{
			LeafName = TagString;
		}

		FAbilityDisplayRow NewRow;
		NewRow.AbilityTag = Tag;
		NewRow.DisplayName = FText::FromString(PascalToDisplayName(LeafName));
		NewRow.Description = FText::GetEmpty();
		NewRow.TintColor = FLinearColor::White;
		NewRow.ActionPointCost = 1;
		NewRow.SortOrder = 0;
		NewRow.bShowInActionBar = true;

		// Use the full tag string as the row name
		const FName RowName(*TagString);
		DisplayTable->AddRow(RowName, NewRow);
		++AddedCount;

		UE_LOG(LogTemp, Log, TEXT("AutoPopulate: Added row '%s' → \"%s\""),
			*TagString, *NewRow.DisplayName.ToString());
	}

	if (AddedCount > 0)
	{
		DisplayTable->Modify();
		UE_LOG(LogTemp, Log, TEXT("AutoPopulateFromGameplayTags: Added %d new row(s) to '%s'."),
			AddedCount, *DisplayTable->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("AutoPopulateFromGameplayTags: No new leaf tags found. Table is up to date."));
	}

	return AddedCount;
}

#endif // WITH_EDITOR
