#pragma once
#include "CoreMinimal.h"
#include "RItemDefinition.h"
#include "LootType.generated.h"

class URItemDefinition;

USTRUCT(BlueprintType)
struct RIZZGAME_API FLootEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TObjectPtr<URItemDefinition> Item = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1", UIMin = "1"))
	int32 Quantity = 1;
};

USTRUCT(BlueprintType)
struct RIZZGAME_API FLootSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (TitleProperty = "Item"))
	TArray<FLootEntry> Entries;
};