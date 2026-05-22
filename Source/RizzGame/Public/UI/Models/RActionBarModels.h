#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RActionBarModels.generated.h"

UENUM(BlueprintType)
enum class EActionSlotState : uint8
{
	Available,
	OnCooldown,
	InsufficientAP,
	Blocked,
	Empty
};

USTRUCT(BlueprintType)
struct FActionSlotViewModel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TintColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ActionPointCost = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EActionSlotState SlotState = EActionSlotState::Empty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CooldownTurnsRemaining = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresTarget = false;
};
