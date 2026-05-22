// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RInventoryItemWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/RInventoryComponent.h"
#include "Items/LootType.h"
#include "RStorageBoxWidget.generated.h"

class UPanelWidget;
class UInventoryItemWidget;
class ARLootContainer;

/**
 * Storage / loot container UI.
 * Shows items from an ARLootContainer using the same row widget as the inventory.
 */
UCLASS()
class RIZZGAME_API URStorageBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Call once after creating the widget. */
	UFUNCTION(BlueprintCallable, Category = "Rizz|Storage")
	void Init(ARLootContainer* InContainer);

	/** Called when inventory changes. */
	UFUNCTION()
	void HandleStorageChanged(ARLootContainer* ChangedContainer);
	
	/** Can be called manually (e.g. after Take All) to refresh the list. */
	UFUNCTION(BlueprintCallable, Category = "Rizz|Storage")
	void RebuildItems();

	UFUNCTION(BlueprintCallable, Category = "Rizz|Storage")
	ARLootContainer* GetBoundContainer() const { return Container; }
	
protected:
	/** Root panel that will contain item widgets (VerticalBox, ScrollBox etc.). */
	UPROPERTY(meta = (BindWidget))
	UPanelWidget* ItemsRoot;

	/** Row widget class; ideally the same as your inventory row (e.g. WBP_InventoryItem). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage")
	TSubclassOf<URInventoryItemWidget> ItemWidgetClass;

	/** Bound loot container. */
	UPROPERTY()
	ARLootContainer* Container;
};
