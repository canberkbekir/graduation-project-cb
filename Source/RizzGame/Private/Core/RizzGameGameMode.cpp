// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/RizzGameGameMode.h"
#include "Core/RCharacterBase.h"
#include "Subsystems/RPartySubsystem.h"
#include "Subsystems/RFogOfWarSubsystem.h"
#include "Engine/World.h"

ARizzGameGameMode::ARizzGameGameMode()
{
	// stub
}

void ARizzGameGameMode::StartPlay()
{
	Super::StartPlay();

	if (UWorld* World = GetWorld())
	{
		if (bFogOfWarEnabled)
		{
			if (URFogOfWarSubsystem* Fog = World->GetSubsystem<URFogOfWarSubsystem>())
			{
				Fog->SpawnLevelFogVolume();
			}
		}
	}
}

void ARizzGameGameMode::RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot)
{
	Super::RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
	ARCharacterBase* Character = GetWorld()->SpawnActor<ARCharacterBase>(
		PlayerCharacter, StartSpot->GetActorTransform());

	if (URPartySubsystem* PartySubsystem = GetGameInstance()->GetSubsystem<URPartySubsystem>())
	{
		PartySubsystem->AddToParty(Character);
		PartySubsystem->SetSelectedCharacter(Character);
	}
}
