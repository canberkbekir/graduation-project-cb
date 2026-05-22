// CameraWallEvents.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CameraWallEvents.generated.h"

/** Toggle a single camera wall by its unique ID. */
USTRUCT(BlueprintType)
struct FCameraWallToggled
{
	GENERATED_BODY()

	UPROPERTY()
	FName WallId = NAME_None;

	UPROPERTY()
	bool bEnabled = false;
};

/** Toggle all camera walls that share the same group tag. */
USTRUCT(BlueprintType)
struct FCameraWallGroupToggled
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag WallGroup;

	UPROPERTY()
	bool bEnabled = false;
};
