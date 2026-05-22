// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/RCombatManagerSubsystem.h"

#include "AbilitySystemComponent.h"
#include "Subsystems/RPartySubsystem.h"
#include "Core/RCharacterBase.h"
#include "Subsystems/REventBusSubsystem.h"
#include "Events/CombatEvents.h"
#include "GAS/Attributes/RCharacterCoreAttributeSet.h"
#include "GAS/Attributes/RCharacterCombatAttributeSet.h"
#include "GAS/RStatusEffectLibrary.h"

void URCombatManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Combatants.Empty();
}

void URCombatManagerSubsystem::AddToCombatants(URTurnComponent* Combatant)
{
	if (Combatant && !Combatants.Contains(Combatant))
	{
		Combatants.Add(Combatant);
	}
}

void URCombatManagerSubsystem::RemoveFromCombat(URTurnComponent* Combatant)
{
	if (!Combatant)
	{
		return;
	}

	const int32 RemovedIndex = Combatants.IndexOfByKey(Combatant);
	if (RemovedIndex == INDEX_NONE)
	{
		return;
	}

	// Let the combatant exit combat state
	Combatant->ExitCombat();

	const bool bWasActiveTurn = (RemovedIndex == CurrentCombatantIndex);

	Combatants.RemoveAt(RemovedIndex);

	if (Combatants.Num() == 0)
	{
		EndCombat();
		return;
	}

	// Adjust CurrentCombatantIndex after removal
	if (bWasActiveTurn)
	{
		// The removed combatant was the active one — the next combatant
		// shifts into the same index, so just clamp/wrap without incrementing.
		if (CurrentCombatantIndex >= Combatants.Num())
		{
			CurrentCombatantIndex = 0;
			++RoundNumber;
		}

		// Start the new current combatant's turn
		URTurnComponent* NextComp = Combatants[CurrentCombatantIndex];
		if (NextComp)
		{
			NextComp->StartTurn();
			if (URPartySubsystem* PartySubsystem = GetGameInstance()->GetSubsystem<URPartySubsystem>())
			{
				PartySubsystem->SetSelectedCharacter(Cast<ARCharacterBase>(NextComp->GetOwner()));
			}

			if (UREventBusSubsystem* Bus = GetGameInstance()->GetSubsystem<UREventBusSubsystem>())
			{
				FTurnStarted StartEvt;
				if (const AActor* Owner = NextComp->GetOwner())
				{
					StartEvt.ActiveActorId = Owner->GetFName();
				}
				Bus->Publish(StartEvt);
				OnTurnStarted.Broadcast(StartEvt.ActiveActorId);
			}
		}
	}
	else if (RemovedIndex < CurrentCombatantIndex)
	{
		// Removed someone before the active combatant — shift index back
		--CurrentCombatantIndex;
	}

	PublishInitiativeEvent();
}

void URCombatManagerSubsystem::EnterCombat()
{
	CalculateInitiative();
	Combatants.Sort([](const URTurnComponent& A, const URTurnComponent& B)
	{
		return A.Initiative > B.Initiative;
	});

	for (URTurnComponent* Combatant : Combatants)
	{
		Combatant->EnterCombat();
	}
	CurrentCombatantIndex = 0;
	RoundNumber = 1;

	if (Combatants.Num() == 0 || !Combatants[CurrentCombatantIndex])
	{
		return;
	}

	Combatants[CurrentCombatantIndex]->StartTurn();
	if (URPartySubsystem* PartySubsystem = GetGameInstance()->GetSubsystem<URPartySubsystem>())
	{
		PartySubsystem->SetSelectedCharacter(Cast<ARCharacterBase>(Combatants[CurrentCombatantIndex]->GetOwner()));
	}

	if (UREventBusSubsystem* Bus = GetGameInstance()->GetSubsystem<UREventBusSubsystem>())
	{
		FCombatStarted Started;
		Started.EncounterId = ActiveEncounterId;
		Bus->Publish(Started);

		FTurnStarted TurnEvt;
		if (const AActor* Owner = Combatants[CurrentCombatantIndex]->GetOwner())
		{
			TurnEvt.ActiveActorId = Owner->GetFName();
		}
		Bus->Publish(TurnEvt);

		OnCombatStarted.Broadcast(ActiveEncounterId);
		OnTurnStarted.Broadcast(TurnEvt.ActiveActorId);
	}

	BindAttributeListeners();

	PublishInitiativeEvent();
}

void URCombatManagerSubsystem::AdvanceTurn()
{
	if (Combatants.Num() == 0 || CurrentCombatantIndex >= Combatants.Num())
	{
		return;
	}

	// Tick EveryTurn effects on ALL combatants before advancing
	for (URTurnComponent* Comp : Combatants)
	{
		if (const auto* Char = Cast<ARCharacterBase>(Comp->GetOwner()))
		{
			if (UAbilitySystemComponent* ASC = Char->GetAbilitySystemComponent())
			{
				URStatusEffectLibrary::TickTurnEffects(ASC, StatusEffectDisplayTable, EREffectTickMode::EveryTurn);
			}
		}
	}

	// Publish FTurnEnded for current combatant
	if (URTurnComponent* CurrentComp = Combatants[CurrentCombatantIndex])
	{
		if (UREventBusSubsystem* Bus = GetGameInstance()->GetSubsystem<UREventBusSubsystem>())
		{
			FTurnEnded EndEvt;
			if (const AActor* Owner = CurrentComp->GetOwner())
			{
				EndEvt.ActiveActorId = Owner->GetFName();
			}
			Bus->Publish(EndEvt);
			OnTurnEnded.Broadcast(EndEvt.ActiveActorId);
		}
	}

	CurrentCombatantIndex = (CurrentCombatantIndex + 1) % Combatants.Num();

	if (CurrentCombatantIndex == 0)
	{
		++RoundNumber;
	}

	URTurnComponent* NextComp = Combatants[CurrentCombatantIndex];
	if (!NextComp)
	{
		return;
	}

	NextComp->StartTurn();
	if (URPartySubsystem* PartySubsystem = GetGameInstance()->GetSubsystem<URPartySubsystem>())
	{
		PartySubsystem->SetSelectedCharacter(Cast<ARCharacterBase>(NextComp->GetOwner()));
	}

	// Tick OnOwnTurnOnly effects on the new active combatant
	if (const auto* Char = Cast<ARCharacterBase>(NextComp->GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = Char->GetAbilitySystemComponent())
		{
			URStatusEffectLibrary::TickTurnEffects(ASC, StatusEffectDisplayTable, EREffectTickMode::OnOwnTurnOnly);
		}
	}

	// Publish FTurnStarted for new combatant
	if (UREventBusSubsystem* Bus = GetGameInstance()->GetSubsystem<UREventBusSubsystem>())
	{
		FTurnStarted StartEvt;
		if (const AActor* Owner = NextComp->GetOwner())
		{
			StartEvt.ActiveActorId = Owner->GetFName();
		}
		Bus->Publish(StartEvt);
		OnTurnStarted.Broadcast(StartEvt.ActiveActorId);
	}

	PublishInitiativeEvent();
}

void URCombatManagerSubsystem::EndCombat()
{
	UnbindAttributeListeners();

	for (URTurnComponent* Combatant : Combatants)
	{
		Combatant->ExitCombat();
	}
	//Combatants.Empty();
	CurrentCombatantIndex = -1;
	RoundNumber = 1;

	if (UREventBusSubsystem* Bus = GetGameInstance()->GetSubsystem<UREventBusSubsystem>())
	{
		FCombatEnded Ended;
		Ended.EncounterId = ActiveEncounterId;
		Bus->Publish(Ended);
		OnCombatEnded.Broadcast(ActiveEncounterId);
	}
}

void URCombatManagerSubsystem::ClearCombatants()
{
	UnbindAttributeListeners();
	Combatants.Empty();
}

void URCombatManagerSubsystem::CalculateInitiative()
{
	for (URTurnComponent* TurnComp : Combatants)
	{
		if (const ARCharacterBase* Character = Cast<ARCharacterBase>(TurnComp->GetOwner()))
		{
			if (const UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
			{
				const float Insight = ASC->GetNumericAttribute(URCharacterCoreAttributeSet::GetInsightAttribute());
				const int32 Roll = FMath::RandRange(1, 20);
				TurnComp->Initiative = Insight + Roll;
			}
		}
	}
}

void URCombatManagerSubsystem::OnAttributeChanged(const FOnAttributeChangeData& Data)
{
	PublishInitiativeEvent();
}

void URCombatManagerSubsystem::OnEffectAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle)
{
	PublishInitiativeEvent();
}

void URCombatManagerSubsystem::OnEffectRemoved(const FActiveGameplayEffect& RemovedEffect)
{
	PublishInitiativeEvent();
}

void URCombatManagerSubsystem::BindAttributeListeners()
{
	UnbindAttributeListeners();

	for (URTurnComponent* TurnComp : Combatants)
	{
		if (!TurnComp)
		{
			continue;
		}

		ARCharacterBase* Character = Cast<ARCharacterBase>(TurnComp->GetOwner());
		if (!Character)
		{
			continue;
		}

		UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
		if (!ASC)
		{
			continue;
		}

		// Bind attribute change delegates for Health, KineticShields, EnergyShields
		AttributeDelegateHandles.Add(
			ASC->GetGameplayAttributeValueChangeDelegate(URCharacterCombatAttributeSet::GetHealthAttribute())
			.AddUObject(this, &URCombatManagerSubsystem::OnAttributeChanged));

		AttributeDelegateHandles.Add(
			ASC->GetGameplayAttributeValueChangeDelegate(URCharacterCombatAttributeSet::GetKineticShieldsAttribute())
			.AddUObject(this, &URCombatManagerSubsystem::OnAttributeChanged));

		AttributeDelegateHandles.Add(
			ASC->GetGameplayAttributeValueChangeDelegate(URCharacterCombatAttributeSet::GetEnergyShieldsAttribute())
			.AddUObject(this, &URCombatManagerSubsystem::OnAttributeChanged));

		// Bind effect added/removed delegates
		EffectAddedHandles.Add(
			ASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &URCombatManagerSubsystem::OnEffectAdded));

		EffectRemovedHandles.Add(
			ASC->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &URCombatManagerSubsystem::OnEffectRemoved));
	}
}

void URCombatManagerSubsystem::UnbindAttributeListeners()
{
	for (URTurnComponent* TurnComp : Combatants)
	{
		if (!TurnComp)
		{
			continue;
		}

		ARCharacterBase* Character = Cast<ARCharacterBase>(TurnComp->GetOwner());
		if (!Character)
		{
			continue;
		}

		UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
		if (!ASC)
		{
			continue;
		}

		// Remove attribute change delegates
		ASC->GetGameplayAttributeValueChangeDelegate(URCharacterCombatAttributeSet::GetHealthAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(URCharacterCombatAttributeSet::GetKineticShieldsAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(URCharacterCombatAttributeSet::GetEnergyShieldsAttribute()).RemoveAll(this);

		// Remove effect delegates
		ASC->OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll(this);
		ASC->OnAnyGameplayEffectRemovedDelegate().RemoveAll(this);
	}

	AttributeDelegateHandles.Empty();
	EffectAddedHandles.Empty();
	EffectRemovedHandles.Empty();
}

void URCombatManagerSubsystem::PublishInitiativeEvent()
{
	UREventBusSubsystem* Bus = GetGameInstance()->GetSubsystem<UREventBusSubsystem>();
	if (!Bus)
	{
		return;
	}

	FCombatInitiativeChanged Evt;
	Evt.CurrentTurnIndex = CurrentCombatantIndex;
	Evt.RoundNumber = RoundNumber;

	for (const URTurnComponent* TurnComp : Combatants)
	{
		const AActor* Owner = TurnComp->GetOwner();
		if (!Owner)
		{
			continue;
		}

		FInitiativeCombatantInfo Info;
		Info.ActorId = Owner->GetFName();
		Info.Team = TurnComp->Team;

		if (const ARCharacterBase* Character = Cast<ARCharacterBase>(Owner))
		{
			Info.Portrait = Character->GetPortrait();

			if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
			{
				Info.CurrentHealth = FMath::RoundToInt32(ASC->GetNumericAttribute(URCharacterCombatAttributeSet::GetHealthAttribute()));
				Info.MaxHealth = FMath::RoundToInt32(ASC->GetNumericAttribute(URCharacterCombatAttributeSet::GetMaxHealthAttribute()));
				Info.CurrentKineticShields = ASC->GetNumericAttribute(URCharacterCombatAttributeSet::GetKineticShieldsAttribute());
				Info.MaxKineticShields = ASC->GetNumericAttribute(URCharacterCombatAttributeSet::GetMaxKineticShieldsAttribute());
				Info.CurrentEnergyShields = ASC->GetNumericAttribute(URCharacterCombatAttributeSet::GetEnergyShieldsAttribute());
				Info.MaxEnergyShields = ASC->GetNumericAttribute(URCharacterCombatAttributeSet::GetMaxEnergyShieldsAttribute());
				Info.StatusEffects = URStatusEffectLibrary::GetActiveEffectIcons(ASC, StatusEffectDisplayTable);
			}
		}

		Evt.Combatants.Add(Info);
	}

	Bus->Publish(Evt);
	OnInitiativeChanged.Broadcast(Evt);
}
