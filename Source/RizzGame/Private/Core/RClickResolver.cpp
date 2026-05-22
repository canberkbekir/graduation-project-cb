#include "Core/RClickResolver.h"

#include "Core/RGameplayAbilityBase.h"
#include "Core/RPlayerControllerBase.h"
#include "Core/RCharacterBase.h"
#include "UI/Combat/RActionBarWidget.h"
#include "Interaction/RInteractable.h"
#include "Interaction/ISelectable.h"
#include "Components/RSelectionComponent.h"
#include "Components/RTurnComponent.h"
#include "Subsystems/RPartySubsystem.h"
#include "Subsystems/RCombatManagerSubsystem.h"
#include "AbilitySystemComponent.h"

void URClickResolver::Initialize(ARPlayerControllerBase* InController)
{
	Controller = InController;
}

void URClickResolver::SetActionBar(URActionBarWidget* InActionBar)
{
	ActionBarWidget = InActionBar;
}

// ─────────────────────────────── Public ───────────────────────────────

void URClickResolver::Resolve(const FHitResult& Hit)
{
	AActor* HitActor = Hit.GetActor();
	UE_LOG(LogTemp, Log, TEXT("[Click] Hit=%s | HasAbilitySelection=%d | InCombat=%d | PlayerTurn=%d"),
		HitActor ? *HitActor->GetName() : TEXT("None"),
		(ActionBarWidget.IsValid() && ActionBarWidget->HasSelection()) ? 1 : 0,
		IsInCombat() ? 1 : 0,
		IsPlayerTurn() ? 1 : 0);

	if (TryAbilityTarget(Hit))     return;
	if (TryInteractable(HitActor)) return;
	if (TryCharacter(HitActor))   return;
	DoGroundMove(Hit.Location);
}

EMouseCursor::Type URClickResolver::GetCursorFor(const FHitResult& Hit) const
{
	if (ActionBarWidget.IsValid() && ActionBarWidget->HasSelection())
	{
		return EMouseCursor::Crosshairs;
	}

	AActor* HitActor = Hit.GetActor();
	if (!HitActor)
	{
		return EMouseCursor::Default;
	}

	if (HitActor->Implements<URInteractable>())
	{
		return EMouseCursor::Hand;
	}

	if (const ARCharacterBase* Char = Cast<ARCharacterBase>(HitActor))
	{
		if (const URTurnComponent* TurnComp = Char->GetTurnComponent())
		{
			return TurnComp->Team == ERCombatTeam::Enemy
				? EMouseCursor::Crosshairs
				: EMouseCursor::Default;
		}
	}

	return EMouseCursor::Default;
}

// ─────────────────────────────── Private ──────────────────────────────

bool URClickResolver::TryAbilityTarget(const FHitResult& Hit)
{
	if (!ActionBarWidget.IsValid() || !ActionBarWidget->HasSelection())
	{
		return false;
	}

	const ETargetingType TargetType = ActionBarWidget->GetSelectedTargetingType();
	const bool bRequiresTarget = ActionBarWidget->SelectedAbilityRequiresTarget();

	UE_LOG(LogTemp, Log, TEXT("[Click] TryAbilityTarget — TargetType=%d RequiresTarget=%d Hit=%s"),
		(int32)TargetType, bRequiresTarget ? 1 : 0,
		Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("None"));

	if (TargetType == ETargetingType::SingleActor
		|| (TargetType != ETargetingType::AOE && bRequiresTarget))
	{
		if (!Cast<ARCharacterBase>(Hit.GetActor()))
		{
			return true;
		}
	}

	ActionBarWidget->TryExecuteSelectedAbility(Hit);
	return true;
}

bool URClickResolver::TryInteractable(AActor* HitActor)
{
	if (!HitActor || !HitActor->Implements<URInteractable>())
	{
		return false;
	}

	ARCharacterBase* Selected = GetSelectedCharacter();
	if (!Selected)
	{
		return false;
	}

	APawn* SelectedPawn = Cast<APawn>(Selected);
	if (IRInteractable::Execute_CanInteract(HitActor, SelectedPawn))
	{
		Selected->Interact(*HitActor);
	}
	return true;
}

bool URClickResolver::TryCharacter(AActor* HitActor)
{
	ARCharacterBase* Char = Cast<ARCharacterBase>(HitActor);
	if (!Char)
	{
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[Click] TryCharacter — actor=%s InCombat=%d PlayerTurn=%d"),
		*Char->GetName(), IsInCombat() ? 1 : 0, IsPlayerTurn() ? 1 : 0);

	ARPlayerControllerBase* PC = Controller.Get();
	if (!PC)
	{
		return false;
	}

	UGameInstance* GI = PC->GetGameInstance();
	URPartySubsystem* Party = GI->GetSubsystem<URPartySubsystem>();
	if (!Party)
	{
		return false;
	}

	// Party member → select
	if (Party->IsInParty(Char))
	{
		ClearSelectionVisual();
		Party->SetSelectedCharacter(Char);
		ApplySelectionVisual(Char);
		if (Char->Implements<URSelectable>())
		{
			IRSelectable::Execute_OnSelected(Char);
		}
		return true;
	}

	// Enemy
	URTurnComponent* TurnComp = Char->GetTurnComponent();
	if (!TurnComp || TurnComp->Team != ERCombatTeam::Enemy)
	{
		return false;
	}

	if (IsInCombat() && IsPlayerTurn())
	{
		ClearSelectionVisual();
		ApplySelectionVisual(Char);
		if (Char->Implements<URSelectable>())
		{
			IRSelectable::Execute_OnSelected(Char);
		}
	}
	else if (!IsInCombat())
	{
		// Exploration: move toward the enemy
		if (ARCharacterBase* Selected = GetSelectedCharacter())
		{
			Selected->MoveToLocation(HitActor->GetActorLocation());
		}
	}
	return true;
}

void URClickResolver::DoGroundMove(const FVector& Location)
{
	if (IsInCombat())
	{
		if (!IsPlayerTurn())
		{
			UE_LOG(LogTemp, Log, TEXT("[Click] Ground click ignored — not player's turn"));
			return;
		}

		ARCharacterBase* Selected = GetSelectedCharacter();
		if (!Selected)
		{
			return;
		}

		UAbilitySystemComponent* ASC = Selected->GetAbilitySystemComponent();
		if (!ASC)
		{
			return;
		}

		// Activate only the first Walk spec found — TryActivateAbilitiesByTag would fire all matching specs
		// if Walk is inadvertently granted twice.
		static const FGameplayTag WalkTag = FGameplayTag::RequestGameplayTag(FName("Ability.Walk"));

		FGameplayAbilitySpec* WalkSpec = nullptr;
		int32 WalkSpecCount = 0;
		for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (Spec.Ability && Spec.Ability->GetAssetTags().HasTagExact(WalkTag))
			{
				++WalkSpecCount;
				if (!WalkSpec) WalkSpec = &Spec;
			}
		}

		if (WalkSpecCount > 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Click] Walk ability is granted %d times — only first spec will be activated. Check GiveAbility calls."), WalkSpecCount);
		}

		const bool bActivated = WalkSpec ? ASC->TryActivateAbility(WalkSpec->Handle) : false;
		UE_LOG(LogTemp, Log, TEXT("[Click] Ground click in combat — Walk triggered=%d (specs found=%d)"), bActivated ? 1 : 0, WalkSpecCount);
		return;
	}

	if (ARCharacterBase* Selected = GetSelectedCharacter())
	{
		Selected->MoveToLocation(Location);
	}
}

// ─────────────────────────────── Helpers ──────────────────────────────

bool URClickResolver::IsInCombat() const
{
	const ARPlayerControllerBase* PC = Controller.Get();
	if (!PC)
	{
		return false;
	}
	const URCombatManagerSubsystem* Combat = PC->GetGameInstance()->GetSubsystem<URCombatManagerSubsystem>();
	if (!Combat)
	{
		return false;
	}
	return Combat->Combatants.Num() > 0 &&
		   Combat->CurrentCombatantIndex < static_cast<uint8>(Combat->Combatants.Num());
}

bool URClickResolver::IsPlayerTurn() const
{
	const ARPlayerControllerBase* PC = Controller.Get();
	if (!PC)
	{
		return false;
	}
	UGameInstance* GI = PC->GetGameInstance();
	const URCombatManagerSubsystem* Combat = GI->GetSubsystem<URCombatManagerSubsystem>();
	const URPartySubsystem* Party = GI->GetSubsystem<URPartySubsystem>();

	if (!Combat || !Party || !IsInCombat())
	{
		return false;
	}

	const URTurnComponent* ActiveTurnComp = Combat->Combatants[Combat->CurrentCombatantIndex];
	if (!ActiveTurnComp)
	{
		return false;
	}

	const ARCharacterBase* ActiveCharacter = Cast<ARCharacterBase>(ActiveTurnComp->GetOwner());
	return ActiveCharacter && const_cast<URPartySubsystem*>(Party)->IsInParty(const_cast<ARCharacterBase*>(ActiveCharacter));
}

ARCharacterBase* URClickResolver::GetSelectedCharacter() const
{
	const ARPlayerControllerBase* PC = Controller.Get();
	if (!PC)
	{
		return nullptr;
	}
	const URPartySubsystem* Party = PC->GetGameInstance()->GetSubsystem<URPartySubsystem>();
	return Party ? Party->GetSelectedCharacter() : nullptr;
}

void URClickResolver::ClearSelectionVisual()
{
	ARCharacterBase* Current = GetSelectedCharacter();
	if (!Current)
	{
		return;
	}

	if (URSelectionComponent* SelComp = Current->FindComponentByClass<URSelectionComponent>())
	{
		SelComp->SetVisualState(ESelectionVisualState::None);
	}
	if (Current->Implements<URSelectable>())
	{
		IRSelectable::Execute_OnDeselected(Current);
	}
}

void URClickResolver::ApplySelectionVisual(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}
	if (URSelectionComponent* SelComp = Actor->FindComponentByClass<URSelectionComponent>())
	{
		SelComp->SetVisualState(ESelectionVisualState::Selected);
	}
}
