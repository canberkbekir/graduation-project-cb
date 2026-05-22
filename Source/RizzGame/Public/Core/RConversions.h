#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RConversions.generated.h"

/**
 * Unit conversion helpers for RizzGame.
 * UE uses centimetres (1 UU = 1 cm) as its world unit.
 * Character stats (walk distance, range) are authored in feet for designer readability.
 */
struct RConversions
{
	static constexpr float CmPerFoot = 30.48f;

	/** Feet → UE units (cm). Use when storing an authored foot value into a world-space attribute. */
	static constexpr float FeetToUU(float Feet) { return Feet * CmPerFoot; }

	/** UE units (cm) → Feet. Use when displaying a world-space distance in the UI. */
	static constexpr float UUToFeet(float UU) { return UU / CmPerFoot; }
};

/** Blueprint-accessible wrappers around RConversions. */
UCLASS()
class RIZZGAME_API URConversionsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Converts feet to UE units (cm). Use to turn a designer-facing foot value into a world distance. */
	UFUNCTION(BlueprintPure, Category="Conversions", meta=(Keywords="feet uu cm convert"))
	static float FeetToUU(float Feet) { return RConversions::FeetToUU(Feet); }

	/** Converts UE units (cm) to feet. Use when displaying a world distance in the UI. */
	UFUNCTION(BlueprintPure, Category="Conversions", meta=(Keywords="feet uu cm convert"))
	static float UUToFeet(float UU) { return RConversions::UUToFeet(UU); }
};
