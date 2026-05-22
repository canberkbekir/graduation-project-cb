// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RizzGameGameMode.generated.h"

class ARCharacterBase;
/**
 *  Simple Game Mode for a top-down perspective game
 *  Sets the default gameplay framework classes
 *  Check the Blueprint derived class for the set values
 */
UCLASS(abstract)
class ARizzGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** Constructor */
	ARizzGameGameMode();
	virtual void StartPlay() override;
	virtual void RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot) override;

	/** The default Character class used by players. */
	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<ARCharacterBase> PlayerCharacter;

	/** Whether the Fog of War system is active for this game mode. */
	UPROPERTY(EditDefaultsOnly, Category = "Fog of War")
	bool bFogOfWarEnabled = true;
};
