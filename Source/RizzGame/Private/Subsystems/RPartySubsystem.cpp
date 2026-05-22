// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/RPartySubsystem.h"
#include "Subsystems/REventBusSubsystem.h"
#include "Core/RCharacterBase.h"

void URPartySubsystem::AddToParty(ARCharacterBase* Character)
{
	if (!PartyMembers.Contains(Character))
	{
		PartyMembers.Add(Character);
	}
}

void URPartySubsystem::RemoveFromParty(ARCharacterBase* Character)
{
	PartyMembers.Remove(Character);
}

void URPartySubsystem::SetSelectedCharacter(ARCharacterBase* Character)
{
	if (PartyMembers.Contains(Character) && SelectedCharacter != Character)
	{
		ARCharacterBase* Previous = SelectedCharacter;
		SelectedCharacter = Character;

		if (auto* Bus = GetGameInstance()->GetSubsystem<UREventBusSubsystem>())
		{
			FSelectedCharacterChanged Event;
			Event.NewCharacter = Cast<AActor>(SelectedCharacter.Get());
			Event.PreviousCharacter = Cast<AActor>(Previous);
			Bus->Publish(Event);
		}
	}
}

ARCharacterBase* URPartySubsystem::GetSelectedCharacter() const
{
	return SelectedCharacter;
}

bool URPartySubsystem::IsInParty(ARCharacterBase* Character)
{
	return PartyMembers.Contains(Character);
}
