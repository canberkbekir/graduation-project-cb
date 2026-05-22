#pragma once
#include "CoreMinimal.h"
#include "CoverEnumsStructs.generated.h"

UENUM(BlueprintType)
enum class ECoverType : uint8
{
	CrouchCover UMETA(DisplayName="CrouchCover"),
	StandCover UMETA(DisplayName="StandCover")
};

UENUM(BlueprintType)
enum class ECrouchCoverPositions : uint8
{
	Idle UMETA(DisplayName="Idle"),
	Up UMETA(DisplayName="Up"),
	Right UMETA(DisplayName="Right"),
	Left UMETA(DisplayName="Left")
};

UENUM(BlueprintType)
enum class EStandCoverPositions : uint8
{
	Idle UMETA(DisplayName="Idle"),
	Right UMETA(DisplayName="Right"),
	Left UMETA(DisplayName="Left")
};