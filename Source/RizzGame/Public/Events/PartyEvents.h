#pragma once

#include "CoreMinimal.h"
#include "PartyEvents.generated.h"

/** Broadcast when the selected party character changes */
USTRUCT()
struct FSelectedCharacterChanged
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> NewCharacter;

	UPROPERTY()
	TWeakObjectPtr<AActor> PreviousCharacter;
};
