// RCooldownGameplayEffect.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "RCooldownGameplayEffect.generated.h"

/**
 * Base class for ability cooldown effects.
 * Designers create a Blueprint child (e.g. GE_Cooldown_MagneticDestabilizer),
 * set Duration Policy = Has Duration, Scalable Float = N turns, then assign
 * it to the ability's CooldownGameplayEffectClass. GAS handles the rest.
 */
UCLASS(BlueprintType, Blueprintable)
class RIZZGAME_API URCooldownGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()
};
