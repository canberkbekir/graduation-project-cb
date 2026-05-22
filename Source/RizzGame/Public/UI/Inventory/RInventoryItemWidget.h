// RInventoryItemWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/RInventoryComponent.h"
#include "RInventoryItemWidget.generated.h"

class UImage;
class UTextBlock;
 
UCLASS()
class RIZZGAME_API URInventoryItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Setup this slot as a filled inventory entry. */
	UFUNCTION(BlueprintCallable, Category = "Rizz|Inventory")
	void SetupFromEntry(const FRInventoryEntry& InEntry);

	/** Setup this slot as empty, at a given logical slot index. */
	UFUNCTION(BlueprintCallable, Category = "Rizz|Inventory")
	void SetupEmptySlot(int32 SlotIndex);

	/** Set the index of this slot in the owning inventory component. */
	UFUNCTION(BlueprintCallable, Category = "Rizz|Inventory")
	void SetInventoryIndex(int32 InIndex);

	/** Current entry data used by the widget. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FRInventoryEntry Entry;

	/** Index inside URInventoryComponent::Entries (or logical slot index). */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 InventoryIndex = INDEX_NONE;

protected: 
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IconImage;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* NameText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* QuantityText;
};
