// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/RStorageBoxWidget.h"

#include "Components/PanelWidget.h"
#include "Components/RInventoryComponent.h"
#include "Gameplay/RLootContainer.h"
#include "Items/LootType.h"

void URStorageBoxWidget::Init(ARLootContainer* InContainer)
{
	Container = InContainer;
	RebuildItems();
}

void URStorageBoxWidget::HandleStorageChanged(ARLootContainer* ChangedContainer)
{
	if (ChangedContainer != Container) { return; }

	RebuildItems();
}

void URStorageBoxWidget::RebuildItems()
{
	if (!ItemsRoot || !ItemWidgetClass || !Container) { return; }

	ItemsRoot->ClearChildren();

	// Get loot entries from the container
	TArray<FLootEntry> LootEntries;
	Container->GetLootEntries(LootEntries);

	for (const auto& [Item, Quantity] : LootEntries) {
		FRInventoryEntry TmpEntry;

		TmpEntry.ItemDefinition = Item.Get();
		TmpEntry.Quantity = Quantity;

		if (!TmpEntry.IsValidEntry()) { continue; }


		URInventoryItemWidget* Row = CreateWidget<URInventoryItemWidget>(this, ItemWidgetClass);
		if (!Row) { continue; }

		Row->SetupFromEntry(TmpEntry);
		ItemsRoot->AddChild(Row);
	}
}
