// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/RCombatScenarioSubsystem.h"
#include "Gameplay/RCombatScenario.h"
#include "Subsystems/RCombatManagerSubsystem.h"
#include "Subsystems/RPartySubsystem.h"
#include "Subsystems/REventBusSubsystem.h"
#include "Events/CombatEvents.h"
#include "Core/RCharacterBase.h"
#include "Components/RTurnComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/RCharacterCombatAttributeSet.h"

void URCombatScenarioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (const UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UREventBusSubsystem* Bus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			CombatEndedHandle = Bus->Subscribe<FCombatEnded>(this, &URCombatScenarioSubsystem::HandleCombatEnded);
		}
	}
}

void URCombatScenarioSubsystem::Deinitialize()
{
	if (const UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UREventBusSubsystem* Bus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			Bus->Unsubscribe<FCombatEnded>(CombatEndedHandle);
		}
	}

	Super::Deinitialize();
}

void URCombatScenarioSubsystem::RegisterScenario(ARCombatScenario* Scenario)
{
	if (Scenario && !RegisteredScenarios.Contains(Scenario))
	{
		RegisteredScenarios.Add(Scenario);
	}
}

void URCombatScenarioSubsystem::UnregisterScenario(ARCombatScenario* Scenario)
{
	RegisteredScenarios.Remove(Scenario);

	if (ActiveScenario == Scenario)
	{
		ActiveScenario = nullptr;
	}
}

bool URCombatScenarioSubsystem::StartScenario(FGameplayTag ScenarioId)
{
	ARCombatScenario* Scenario = FindScenario(ScenarioId);
	if (!Scenario)
	{
		return false;
	}

	return StartScenarioInternal(Scenario);
}

bool URCombatScenarioSubsystem::StartScenarioByRef(ARCombatScenario* Scenario)
{
	if (!Scenario)
	{
		return false;
	}

	return StartScenarioInternal(Scenario);
}

ARCombatScenario* URCombatScenarioSubsystem::GetActiveScenario() const
{
	return ActiveScenario;
}

bool URCombatScenarioSubsystem::IsInScenario() const
{
	return ActiveScenario != nullptr;
}

ARCombatScenario* URCombatScenarioSubsystem::FindScenario(FGameplayTag ScenarioId) const
{
	for (ARCombatScenario* Scenario : RegisteredScenarios)
	{
		if (Scenario && Scenario->ScenarioId.MatchesTagExact(ScenarioId))
		{
			return Scenario;
		}
	}
	return nullptr;
}

void URCombatScenarioSubsystem::OnPreCombatFinished(ARCombatScenario* Scenario)
{
	if (Scenario != ActiveScenario)
	{
		return;
	}

	BeginCombatForScenario(Scenario);
}

void URCombatScenarioSubsystem::OnPostCombatFinished(ARCombatScenario* Scenario)
{
	if (Scenario != ActiveScenario)
	{
		return;
	}

	Scenario->MarkCompleted();
	ActiveScenario = nullptr;
}

bool URCombatScenarioSubsystem::StartScenarioInternal(ARCombatScenario* Scenario)
{
	if (ActiveScenario)
	{
		return false;
	}

	if (Scenario->ScenarioState == ERCombatScenarioState::Completed && !Scenario->bRepeatable)
	{
		return false;
	}

	ActiveScenario = Scenario;

	if (Scenario->bUsePreCombatPhase)
	{
		Scenario->ExecutePreCombat();
	}
	else
	{
		BeginCombatForScenario(Scenario);
	}

	return true;
}

void URCombatScenarioSubsystem::BeginCombatForScenario(ARCombatScenario* Scenario)
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		return;
	}

	URCombatManagerSubsystem* CombatManager = GI->GetSubsystem<URCombatManagerSubsystem>();
	if (!CombatManager)
	{
		return;
	}

	CombatManager->ClearCombatants();

	// Collect enemies directly referenced in the level
	for (ARCharacterBase* Enemy : Scenario->Enemies)
	{
		if (!Enemy)
		{
			continue;
		}

		if (URTurnComponent* TurnComp = Enemy->GetTurnComponent())
		{
			CombatManager->AddToCombatants(TurnComp);
		}
	}

	// If no valid enemies, complete immediately
	if (CombatManager->Combatants.Num() == 0)
	{
		Scenario->MarkCompleted();
		ActiveScenario = nullptr;
		return;
	}

	// Auto-join party members
	if (Scenario->bAutoJoinParty)
	{
		if (URPartySubsystem* PartySubsystem = GI->GetSubsystem<URPartySubsystem>())
		{
			for (ARCharacterBase* PartyMember : PartySubsystem->PartyMembers)
			{
				if (!PartyMember)
				{
					continue;
				}

				if (URTurnComponent* TurnComp = PartyMember->GetTurnComponent())
				{
					CombatManager->AddToCombatants(TurnComp);
				}
			}
		}
	}

	CombatManager->ActiveEncounterId = Scenario->ScenarioId;
	Scenario->ExecuteStartCombat();
	CombatManager->EnterCombat();
}

void URCombatScenarioSubsystem::HandleCombatEnded(const FCombatEnded& Event)
{
	if (!ActiveScenario)
	{
		return;
	}

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI)
	{
		if (URCombatManagerSubsystem* CombatManager = GI->GetSubsystem<URCombatManagerSubsystem>())
		{
			CombatManager->ClearCombatants();
		}
	}

	if (ActiveScenario->bUsePostCombatPhase)
	{
		ActiveScenario->ExecutePostCombat();
	}
	else
	{
		ActiveScenario->ScenarioState = ERCombatScenarioState::Completed;
		ActiveScenario = nullptr;
	}
}
