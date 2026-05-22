#pragma once
#include "RCharacterCombatAttributeSet.h"
#include "RCharacterCoreAttributeSet.h"
#include "RCharacterTurnAttributeSet.h"
#include "RAttributeSets.generated.h"

USTRUCT(BlueprintType)
struct FRAttributeSets
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<URCharacterCombatAttributeSet> CombatAttributes;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<URCharacterCoreAttributeSet> CoreAttributes;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<URCharacterTurnAttributeSet> TurnAttributes;
	 
};
