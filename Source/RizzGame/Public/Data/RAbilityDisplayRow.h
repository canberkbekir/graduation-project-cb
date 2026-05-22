#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "RAbilityDisplayRow.generated.h"

USTRUCT(BlueprintType)
struct FAbilityDisplayRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Tag that identifies this ability (e.g. Ability.Attack.Melee) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability")
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability")
	FLinearColor TintColor = FLinearColor::White;

	/** Visual AP cost shown in the action bar (actual cost lives in the GE) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability")
	int32 ActionPointCost = 1;

	/** Sort priority (lower = further left) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability")
	int32 SortOrder = 0;

	/** If false, this ability is hidden from the action bar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability")
	bool bShowInActionBar = true;
};
