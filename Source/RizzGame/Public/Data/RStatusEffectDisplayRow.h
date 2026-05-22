#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "RStatusEffectDisplayRow.generated.h"

UENUM(BlueprintType)
enum class EREffectTickMode : uint8
{
	OnOwnTurnOnly   UMETA(DisplayName = "On Own Turn Only"),
	EveryTurn        UMETA(DisplayName = "Every Turn")
};

USTRUCT(BlueprintType)
struct FStatusEffectDisplayRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect")
	FGameplayTag EffectTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect")
	FLinearColor TintColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect")
	bool bShowStacks = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect")
	EREffectTickMode TickMode = EREffectTickMode::OnOwnTurnOnly;
};
