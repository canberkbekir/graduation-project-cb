// RInventoryItemWidget.cpp

#include "UI/Inventory/RInventoryItemWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Items/RItemDefinition.h"

void URInventoryItemWidget::SetupFromEntry(const FRInventoryEntry& InEntry)
{
	Entry = InEntry;

	URItemDefinition* Def = Entry.ItemDefinition;

	// Name
	if (NameText)
	{
		FText Name = FText::GetEmpty();

		if (Def)
		{ 
			Name = FText::FromString(Def->Visual.ItemName.ToString());
		}

		NameText->SetText(Name);
	}

	// Quantity
	if (QuantityText)
	{
		if (Def && Def->bIsStackable)
		{
			QuantityText->SetText(FText::AsNumber(Entry.Quantity));
		}
		else
		{
			QuantityText->SetText(FText::GetEmpty());
		}
	}

	// Icon
	if (IconImage && Entry.ItemDefinition)
	{ 
		UTexture2D* IconTex = Entry.ItemDefinition->Visual.Icon.LoadSynchronous();
		IconImage->SetBrushFromTexture(IconTex, true);
	}
}


void URInventoryItemWidget::SetupEmptySlot(int32 SlotIndex)
{ 
	Entry = FRInventoryEntry();
	InventoryIndex = SlotIndex;
 
	if (NameText) { NameText->SetText(FText::GetEmpty()); }

	if (QuantityText) { QuantityText->SetText(FText::GetEmpty()); }
 
	if (IconImage) {
		IconImage->SetBrushFromTexture(nullptr);
		IconImage->SetColorAndOpacity(FLinearColor::Transparent);
	}
}

void URInventoryItemWidget::SetInventoryIndex(int32 InIndex) { InventoryIndex = InIndex; }