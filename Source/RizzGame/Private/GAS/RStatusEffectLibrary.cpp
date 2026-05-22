#include "GAS/RStatusEffectLibrary.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Engine/DataTable.h"
#include "Subsystems/DiceSubsystem.h"
#include "GAS/Effects/RStatusGameplayEffect.h"
#include "GAS/Attributes/RCharacterCoreAttributeSet.h"

#if WITH_EDITOR
#include "UObject/UObjectIterator.h"
#include "GameplayTagsManager.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogStatusEffects, Log, All);

// Tag used to identify effects that participate in turn-based ticking
static const FName TurnBasedTagName(TEXT("Effect.Status.PerTurn"));
static const FName TurnsRemainingTagName(TEXT("Data.Effect.TurnsLeft"));

static FString DiceExpressionToString(const FDiceExpression& Expr)
{
	if (Expr.IsEmpty()) return TEXT("empty");

	const UEnum* DiceEnum = StaticEnum<EDiceType>();
	FString Result;
	for (int32 i = 0; i < Expr.Terms.Num(); ++i)
	{
		if (i > 0) Result += TEXT("+");
		const FDiceTerm& Term = Expr.Terms[i];
		FString DieName = DiceEnum ? DiceEnum->GetNameStringByValue(static_cast<int64>(Term.Die)) : TEXT("?");
		Result += FString::Printf(TEXT("%d%s"), Term.Count, *DieName);
	}
	if (Expr.FlatModifier != 0)
		Result += FString::Printf(TEXT("%+d"), Expr.FlatModifier);
	return Result;
}

/**
 * Collect all tags associated with an active GE.
 *
 * UE5.3+ moved tag configuration into GE components. UTargetTagsGameplayEffectComponent grants
 * tags as loose tags on the ASC, so they never appear in Spec.GetAllGrantedTags(). We use
 * FindComponent to read them directly from the GE definition.
 */
static FGameplayTagContainer GetEffectTags(const FActiveGameplayEffect& ActiveGE)
{
	FGameplayTagContainer Tags;

	if (const UGameplayEffect* Def = ActiveGE.Spec.Def)
	{
		// Asset tags (UE5.3+ replaces deprecated InheritableGameplayEffectTags)
		Tags.AppendTags(Def->GetAssetTags());

		// Legacy granted tags (pre-5.3)
		Tags.AppendTags(Def->GetGrantedTags());

		// UE5.3+ component: "Grant Tags to Target Actor"
		if (const UTargetTagsGameplayEffectComponent* TargetTagsComp =
			Def->FindComponent<UTargetTagsGameplayEffectComponent>())
		{
			Tags.AppendTags(TargetTagsComp->GetConfiguredTargetTagChanges().CombinedTags);
		}
	}

	// Dynamic tags added to the spec at runtime
	ActiveGE.Spec.GetAllAssetTags(Tags);
	ActiveGE.Spec.GetAllGrantedTags(Tags);

	return Tags;
}

/**
 * Find the first DataTable row whose EffectTag matches any tag in the given container.
 */
static const FStatusEffectDisplayRow* FindDisplayRow(const UDataTable* Table, const FGameplayTagContainer& Tags)
{
	if (!Table)
	{
		return nullptr;
	}

	static const FString ContextString(TEXT("StatusEffectLookup"));
	TArray<FStatusEffectDisplayRow*> Rows;
	Table->GetAllRows(ContextString, Rows);

	for (const FStatusEffectDisplayRow* Row : Rows)
	{
		if (Row && Tags.HasTag(Row->EffectTag))
		{
			return Row;
		}
	}

	return nullptr;
}

TArray<FEffectIconViewModel> URStatusEffectLibrary::GetActiveEffectIcons(
	UAbilitySystemComponent* ASC,
	const UDataTable* DisplayTable)
{
	TArray<FEffectIconViewModel> Result;

	if (!ASC || !DisplayTable)
	{
		return Result;
	}

	TArray<FActiveGameplayEffectHandle> ActiveHandles = ASC->GetActiveEffects(FGameplayEffectQuery());

	for (const FActiveGameplayEffectHandle& Handle : ActiveHandles)
	{
		const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
		if (!ActiveGE)
		{
			continue;
		}

		const FGameplayTagContainer EffectTags = GetEffectTags(*ActiveGE);

		// Strip internal behavior tags (Effect.Status.*) before display matching —
		// they're tick/behavior markers, not identity tags, and would shadow the real effect row.
		static const FGameplayTag StatusRoot =
			FGameplayTag::RequestGameplayTag(FName("Effect.Status"), false);
		FGameplayTagContainer DisplayTags;
		for (const FGameplayTag& Tag : EffectTags)
		{
			if (!Tag.MatchesTag(StatusRoot))
			{
				DisplayTags.AddTag(Tag);
			}
		}

		const FStatusEffectDisplayRow* Row = FindDisplayRow(DisplayTable, DisplayTags);
		if (!Row)
		{
			continue;
		}

		FEffectIconViewModel VM;
		VM.Icon = Row->Icon.LoadSynchronous();
		VM.DisplayName = Row->DisplayName;
		VM.Description = Row->Description;
		VM.TintColor = Row->TintColor;

		// Stack count: use the GE's stack count if available, otherwise 1
		VM.Stacks = ActiveGE->Spec.GetStackCount();
		if (VM.Stacks <= 0)
		{
			VM.Stacks = 1;
		}

		// If this is a turn-based effect, show remaining turns instead of stacks
		const FGameplayTag TurnBasedTag = FGameplayTag::RequestGameplayTag(TurnBasedTagName);
		const FGameplayTag TurnsTag = FGameplayTag::RequestGameplayTag(TurnsRemainingTagName);
		if (EffectTags.HasTag(TurnBasedTag))
		{
			const float Turns = ActiveGE->Spec.GetSetByCallerMagnitude(TurnsTag, false, 0.f);
			if (Turns > 0.f)
			{
				VM.Stacks = FMath::RoundToInt32(Turns);
			}
		}

		if (!Row->bShowStacks)
		{
			VM.Stacks = 0;
		}

		// Read remaining turns from the GE spec (set by TickTurnEffects via SetByCaller)
		const float RemainingTurns = ActiveGE->Spec.GetSetByCallerMagnitude(TurnsTag, false, 0.f);
		VM.TurnsLeft = FMath::RoundToInt32(RemainingTurns);

		Result.Add(VM);
	}

	return Result;
}

static float GetAttributeValueByTag(UAbilitySystemComponent* ASC, FGameplayTag AttributeTag)
{
	if (!ASC || !AttributeTag.IsValid())
		return 0.f;

	static TMap<FGameplayTag, FGameplayAttribute> TagToAttr;
	if (TagToAttr.IsEmpty())
	{
		TagToAttr.Add(FGameplayTag::RequestGameplayTag("Stat.Tech"),
			URCharacterCoreAttributeSet::GetTechAttribute());
		TagToAttr.Add(FGameplayTag::RequestGameplayTag("Stat.Physique"),
			URCharacterCoreAttributeSet::GetPhysiqueAttribute());
		TagToAttr.Add(FGameplayTag::RequestGameplayTag("Stat.Insight"),
			URCharacterCoreAttributeSet::GetInsightAttribute());
		TagToAttr.Add(FGameplayTag::RequestGameplayTag("Stat.Mind"),
			URCharacterCoreAttributeSet::GetMindAttribute());
		TagToAttr.Add(FGameplayTag::RequestGameplayTag("Stat.Finesse"),
			URCharacterCoreAttributeSet::GetFinesseAttribute());
	}

	const FGameplayAttribute* Attr = TagToAttr.Find(AttributeTag);
	if (!Attr) return 0.f;

	bool bFound = false;
	return ASC->GetGameplayAttributeValue(*Attr, bFound);
}

void URStatusEffectLibrary::TickTurnEffects(
	UAbilitySystemComponent* ASC,
	const UDataTable* DisplayTable,
	EREffectTickMode TickMode)
{
	if (!ASC)
	{
		return;
	}

	(void)DisplayTable; // Kept in signature for API compatibility; tick config now lives on GE CDO

	const FGameplayTag TurnBasedTag = FGameplayTag::RequestGameplayTag(TurnBasedTagName);
	const FGameplayTag TurnsTag = FGameplayTag::RequestGameplayTag(TurnsRemainingTagName);

	// Query all active effects — we filter by granted tags below
	TArray<FActiveGameplayEffectHandle> ActiveHandles = ASC->GetActiveEffects(FGameplayEffectQuery());

	const FString OwnerName = ASC->GetOwnerActor() ? ASC->GetOwnerActor()->GetName() : TEXT("Unknown");
	UE_LOG(LogStatusEffects, Log,
		TEXT("[StatusEffect] TickTurnEffects — Owner: %s | Mode: %s | ActiveGEs: %d"),
		*OwnerName,
		TickMode == EREffectTickMode::OnOwnTurnOnly ? TEXT("OnOwnTurnOnly") : TEXT("EveryTurn"),
		ActiveHandles.Num());

	UDiceSubsystem* DiceSys = ASC->GetOwner()->GetGameInstance()->GetSubsystem<UDiceSubsystem>();

	// Collect handles to remove after iteration (avoid modifying while iterating)
	TArray<FActiveGameplayEffectHandle> HandlesToRemove;

	for (const FActiveGameplayEffectHandle& Handle : ActiveHandles)
	{
		const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
		if (!ActiveGE)
		{
			continue;
		}

		const FString GEName = ActiveGE->Spec.Def ? ActiveGE->Spec.Def->GetClass()->GetName() : TEXT("null");

		// Check granted tags for TurnBased marker, then match against display row
		const FGameplayTagContainer EffectTags = GetEffectTags(*ActiveGE);
		if (!EffectTags.HasTag(TurnBasedTag))
		{
			UE_LOG(LogStatusEffects, Log,
				TEXT("[StatusEffect]   SKIP %s — missing tag '%s' (has tags: %s)"),
				*GEName, *TurnBasedTagName.ToString(), *EffectTags.ToStringSimple());
			continue;
		}

		// Read tick config directly from the GE CDO — no DataTable needed
		const URStatusGameplayEffect* StatusGEDef = Cast<URStatusGameplayEffect>(ActiveGE->Spec.Def);
		if (!StatusGEDef)
		{
			UE_LOG(LogStatusEffects, Warning,
				TEXT("[StatusEffect]   SKIP %s — has '%s' tag but is not a URStatusGameplayEffect subclass"),
				*GEName, *TurnBasedTagName.ToString());
			continue;
		}

		// For damage effects, filter by the tick phase (own-turn vs every-turn)
		// Pure modifier debuffs (None) always proceed so their turn counter still decrements
		if (StatusGEDef->DamageTickType != ERDamageTickType::None &&
			StatusGEDef->TickMode != TickMode)
		{
			continue;
		}

		// Read current turns remaining
		float TurnsRemaining = ActiveGE->Spec.GetSetByCallerMagnitude(TurnsTag, false, 0.f);

		const FString EffectName  = StatusGEDef->GetClass()->GetName();
		const FString TargetName  = ASC->GetOwnerActor() ? ASC->GetOwnerActor()->GetName() : TEXT("Unknown");

		UE_LOG(LogStatusEffects, Log,
			TEXT("[StatusEffect] %s → TICK on %s | Type: %s | TurnsLeft: %.0f"),
			*EffectName, *TargetName,
			*StaticEnum<ERDamageTickType>()->GetNameStringByValue(static_cast<int64>(StatusGEDef->DamageTickType)),
			TurnsRemaining);

		// Compute tick damage
		float TickDamage = 0.f;
		bool bShouldDealDamage = false;

		switch (StatusGEDef->DamageTickType)
		{
		case ERDamageTickType::Fixed:
			TickDamage = StatusGEDef->FixedDamageAmount;
			bShouldDealDamage = (TickDamage > 0.f);
			UE_LOG(LogStatusEffects, Log,
				TEXT("[StatusEffect] %s → Fixed damage: %.1f"),
				*EffectName, TickDamage);
			break;

		case ERDamageTickType::Dice:
			if (!StatusGEDef->TickDiceRoll.IsEmpty())
			{
				if (DiceSys)
				{
					FDiceRollResult Roll = DiceSys->RollExpression(
						StatusGEDef->TickDiceRoll, EDiceRollContext::Combat);
					TickDamage = static_cast<float>(Roll.Total);
					bShouldDealDamage = true;
					UE_LOG(LogStatusEffects, Log,
						TEXT("[StatusEffect] %s → Rolled %s = %d (individual: [%s])"),
						*EffectName,
						*DiceExpressionToString(StatusGEDef->TickDiceRoll),
						Roll.Total,
						*FString::JoinBy(Roll.IndividualRolls, TEXT(", "),
							[](int32 v){ return FString::FromInt(v); }));
				}
			}
			else
			{
				UE_LOG(LogStatusEffects, Warning,
					TEXT("[StatusEffect] %s → DamageTickType is Dice but TickDiceRoll is empty — no damage dealt"),
					*EffectName);
			}
			break;

		case ERDamageTickType::StatBased:
			if (StatusGEDef->StatAttributeTag.IsValid())
			{
				float StatValue = GetAttributeValueByTag(ASC, StatusGEDef->StatAttributeTag);
				TickDamage = StatValue * StatusGEDef->StatCoefficient;
				bShouldDealDamage = (TickDamage > 0.f);
				UE_LOG(LogStatusEffects, Log,
					TEXT("[StatusEffect] %s → StatBased: %s=%.1f × %.2f = %.1f damage"),
					*EffectName,
					*StatusGEDef->StatAttributeTag.ToString(),
					StatValue, StatusGEDef->StatCoefficient, TickDamage);
			}
			else
			{
				UE_LOG(LogStatusEffects, Warning,
					TEXT("[StatusEffect] %s → DamageTickType is StatBased but StatAttributeTag is invalid — no damage dealt"),
					*EffectName);
			}
			break;

		case ERDamageTickType::None:
		default:
			UE_LOG(LogStatusEffects, Log,
				TEXT("[StatusEffect] %s → Modifier-only debuff, no damage tick"),
				*EffectName);
			break;
		}

		if (bShouldDealDamage &&
			IsValid(StatusGEDef->DamageEffectClass) &&
			StatusGEDef->DamageSetByCallerTag.IsValid())
		{
			FGameplayEffectContextHandle DamageCtx = ASC->MakeEffectContext();
			DamageCtx.AddInstigator(ASC->GetOwnerActor(), ASC->GetOwnerActor());

			FGameplayEffectSpecHandle DamageSpec = ASC->MakeOutgoingSpec(
				StatusGEDef->DamageEffectClass,
				ActiveGE->Spec.GetLevel(),
				DamageCtx);

			if (DamageSpec.IsValid())
			{
				DamageSpec.Data->SetSetByCallerMagnitude(
					StatusGEDef->DamageSetByCallerTag, TickDamage);
				ASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());

				UE_LOG(LogStatusEffects, Log,
					TEXT("[StatusEffect] %s → Applied %.1f %s damage to %s via %s"),
					*EffectName, TickDamage,
					*StatusGEDef->DamageSetByCallerTag.ToString(),
					*TargetName,
					*StatusGEDef->DamageEffectClass->GetName());
			}
		}
		else if (bShouldDealDamage)
		{
			UE_LOG(LogStatusEffects, Warning,
				TEXT("[StatusEffect] %s → Damage computed (%.1f) but DamageEffectClass or DamageSetByCallerTag not set — check Blueprint config"),
				*EffectName, TickDamage);
		}

		// Decrement turns remaining
		TurnsRemaining -= 1.f;

		if (FActiveGameplayEffect* MutableGE = const_cast<FActiveGameplayEffect*>(ActiveGE))
		{
			MutableGE->Spec.SetSetByCallerMagnitude(TurnsTag, TurnsRemaining);
		}

		UE_LOG(LogStatusEffects, Log,
			TEXT("[StatusEffect] %s on %s → TurnsLeft after tick: %.0f"),
			*EffectName, *TargetName, TurnsRemaining);

		// If turns hit 0 or below, mark for removal
		if (TurnsRemaining <= 0.f)
		{
			UE_LOG(LogStatusEffects, Log,
				TEXT("[StatusEffect] %s → EXPIRED on %s, removing"),
				*EffectName, *TargetName);
			HandlesToRemove.Add(Handle);
		}
	}

	// Remove expired effects
	for (const FActiveGameplayEffectHandle& Handle : HandlesToRemove)
	{
		ASC->RemoveActiveGameplayEffect(Handle);
	}
}

// ────────────── Editor-Only: Auto-Populate Effect DataTable ──────────────

#if WITH_EDITOR

static FString EffectLeafToDisplayName(const FString& PascalString)
{
	FString Result;
	Result.Reserve(PascalString.Len() + 8);
	for (int32 i = 0; i < PascalString.Len(); ++i)
	{
		const TCHAR Ch = PascalString[i];
		if (i > 0 && FChar::IsUpper(Ch) && FChar::IsLower(PascalString[i - 1]))
		{
			Result.AppendChar(TEXT(' '));
		}
		Result.AppendChar(Ch);
	}
	return Result;
}

static void CollectEffectLeafTags(
	const UGameplayTagsManager& Manager,
	const FGameplayTag& ParentTag,
	const FGameplayTag& SkipTag,
	TArray<FGameplayTag>& OutLeafTags)
{
	FGameplayTagContainer Children = Manager.RequestGameplayTagChildren(ParentTag);

	if (Children.Num() == 0)
	{
		OutLeafTags.Add(ParentTag);
		return;
	}

	for (const FGameplayTag& Child : Children)
	{
		// Skip internal subtrees (e.g. Effect.Status)
		if (SkipTag.IsValid() && Child.MatchesTag(SkipTag))
		{
			continue;
		}
		CollectEffectLeafTags(Manager, Child, SkipTag, OutLeafTags);
	}
}

int32 URStatusEffectLibrary::AutoPopulateEffectDisplayTable(UDataTable* DisplayTable)
{
	if (!DisplayTable)
	{
		UE_LOG(LogStatusEffects, Warning, TEXT("AutoPopulateEffectDisplayTable: DisplayTable is null."));
		return 0;
	}

	if (DisplayTable->GetRowStruct() != FStatusEffectDisplayRow::StaticStruct())
	{
		UE_LOG(LogStatusEffects, Error,
			TEXT("AutoPopulateEffectDisplayTable: '%s' row type is not FStatusEffectDisplayRow."),
			*DisplayTable->GetName());
		return 0;
	}

	const UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
	const FGameplayTag EffectRoot = FGameplayTag::RequestGameplayTag(FName(TEXT("Effect")), false);
	const FGameplayTag StatusRoot = FGameplayTag::RequestGameplayTag(FName(TEXT("Effect.Status")), false);

	if (!EffectRoot.IsValid())
	{
		UE_LOG(LogStatusEffects, Warning, TEXT("AutoPopulateEffectDisplayTable: 'Effect' root tag not found."));
		return 0;
	}

	// Collect all leaf tags under Effect.* skipping Effect.Status.*
	TArray<FGameplayTag> LeafTags;
	CollectEffectLeafTags(TagManager, EffectRoot, StatusRoot, LeafTags);

	// Collect tags already in the table to skip duplicates
	TSet<FGameplayTag> ExistingTags;
	{
		static const FString Context(TEXT("AutoPopulateEffect_ExistingCheck"));
		TArray<FStatusEffectDisplayRow*> ExistingRows;
		DisplayTable->GetAllRows(Context, ExistingRows);
		for (const FStatusEffectDisplayRow* Row : ExistingRows)
		{
			if (Row && Row->EffectTag.IsValid())
			{
				ExistingTags.Add(Row->EffectTag);
			}
		}
	}

	int32 AddedCount = 0;

	for (const FGameplayTag& Tag : LeafTags)
	{
		if (ExistingTags.Contains(Tag)) continue;

		const FString TagString = Tag.ToString();
		FString LeafName;
		TagString.Split(TEXT("."), nullptr, &LeafName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		if (LeafName.IsEmpty()) LeafName = TagString;

		FStatusEffectDisplayRow NewRow;
		NewRow.EffectTag   = Tag;
		NewRow.DisplayName = FText::FromString(EffectLeafToDisplayName(LeafName));
		NewRow.Description = FText::GetEmpty();
		NewRow.TintColor   = FLinearColor::White;
		NewRow.bShowStacks = true;

		DisplayTable->AddRow(FName(*TagString), NewRow);
		ExistingTags.Add(Tag);
		++AddedCount;

		UE_LOG(LogStatusEffects, Log, TEXT("AutoPopulateEffect: Added '%s' → \"%s\""),
			*TagString, *NewRow.DisplayName.ToString());
	}

	if (AddedCount > 0)
	{
		DisplayTable->Modify();
		UE_LOG(LogStatusEffects, Log,
			TEXT("AutoPopulateEffectDisplayTable: Added %d new row(s) to '%s'."),
			AddedCount, *DisplayTable->GetName());
	}
	else
	{
		UE_LOG(LogStatusEffects, Log, TEXT("AutoPopulateEffectDisplayTable: Table is up to date."));
	}

	return AddedCount;
}

#endif // WITH_EDITOR
