// FogOfWarEvents.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FogOfWarEvents.generated.h"

USTRUCT(BlueprintType)
struct FRDoorOpened
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag DoorId;

	UPROPERTY()
	TWeakObjectPtr<AActor> InteractingActor;
};

USTRUCT(BlueprintType)
struct FRDoorClosed
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag DoorId;
};

USTRUCT(BlueprintType)
struct FRRoomRevealed
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag RoomId;

	UPROPERTY()
	FGameplayTag RoomGroup;
};

USTRUCT(BlueprintType)
struct FRRoomHidden
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag RoomId;

	UPROPERTY()
	FGameplayTag RoomGroup;
};

USTRUCT(BlueprintType)
struct FRFogGridUpdated
{
	GENERATED_BODY()

	UPROPERTY()
	int32 CellsChanged = 0;
};
