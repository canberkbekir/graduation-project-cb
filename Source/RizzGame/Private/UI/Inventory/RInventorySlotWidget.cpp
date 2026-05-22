// RInventorySlotWidget.cpp

#include "UI/Inventory/RInventorySlotWidget.h"
#include "Items/RItemDefinition.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

URInventorySlotWidget::URInventorySlotWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void URInventorySlotWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    // Apply slot size
    ApplySlotSize();

    // Show editor preview when designing (for standalone use)
    if (IsDesignTime() && bShowEditorPreview)
    {
        SetupAsEditorPreview(0, EditorPreviewQuantity);
    }
    else
    {
        UpdateVisualState();
    }
}

void URInventorySlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Apply slot size
    ApplySlotSize();

    // Bind button click if available
    if (Btn_Slot)
    {
        Btn_Slot->OnClicked.AddDynamic(this, &URInventorySlotWidget::HandleButtonClicked);
    }

    UpdateVisualState();
}

void URInventorySlotWidget::ApplySlotSize()
{
    if (SlotSize > 0 && SizeBoxRoot)
    {
        SizeBoxRoot->SetWidthOverride(SlotSize);
        SizeBoxRoot->SetHeightOverride(SlotSize);
    }
}

void URInventorySlotWidget::NativeDestruct()
{
    if (Btn_Slot)
    {
        Btn_Slot->OnClicked.RemoveDynamic(this, &URInventorySlotWidget::HandleButtonClicked);
    }

    Super::NativeDestruct();
}

void URInventorySlotWidget::SetupFromEntry(const FRInventoryEntry& Entry, int32 InSlotIndex)
{
    CurrentEntry = Entry;
    SlotIndex = InSlotIndex;
    bIsEditorPreview = false;
    PreviewQuantity = 0;
    UpdateVisualState();
}

void URInventorySlotWidget::SetupAsEmpty(int32 InSlotIndex)
{
    CurrentEntry = FRInventoryEntry();
    SlotIndex = InSlotIndex;
    bIsEditorPreview = false;
    PreviewQuantity = 0;
    UpdateVisualState();
}

void URInventorySlotWidget::SetupAsEditorPreview(int32 InSlotIndex, int32 InPreviewQuantity)
{
    CurrentEntry = FRInventoryEntry();
    SlotIndex = InSlotIndex;
    bIsEditorPreview = true;
    PreviewQuantity = InPreviewQuantity;
    UpdateVisualState();
}

void URInventorySlotWidget::SetSelected(bool bInSelected)
{
    if (bIsSelected == bInSelected)
    {
        return;
    }

    bIsSelected = bInSelected;
    ApplyRarityBorder();
}

bool URInventorySlotWidget::HasDisplayableContent() const
{
    return CurrentEntry.IsValidEntry() || bIsEditorPreview;
}

void URInventorySlotWidget::SetIconBrush(const FSlateBrush& NewBrush)
{
    if (IMG_Icon)
    {
        IMG_Icon->SetBrush(NewBrush);
    }
}

void URInventorySlotWidget::UpdateVisualState()
{
    // Determine what icon to show
    if (CurrentEntry.IsValidEntry() && CurrentEntry.ItemDefinition)
    {
        // Show actual item icon
        if (IMG_Icon)
        {
            if (UTexture2D* Icon = CurrentEntry.ItemDefinition->Visual.Icon.LoadSynchronous())
            {
                FSlateBrush ItemBrush;
                ItemBrush.SetResourceObject(Icon);
                ItemBrush.ImageSize = FVector2D(64.0f, 64.0f);
                SetIconBrush(ItemBrush);
            }
            else
            {
                SetIconBrush(EmptySlotIcon);
            }
        }

        // Show item name if text block exists
        if (Txt_ItemName)
        {
            Txt_ItemName->SetText(CurrentEntry.ItemDefinition->Visual.ItemName);
            Txt_ItemName->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
    }
    else if (bIsEditorPreview)
    {
        // Show preview icon
        if (IMG_Icon)
        {
            // Use preview icon if set, otherwise use empty slot icon
            if (PreviewItemIcon.GetResourceObject())
            {
                SetIconBrush(PreviewItemIcon);
            }
            else
            {
                SetIconBrush(EmptySlotIcon);
            }
        }

        // Show preview text
        if (Txt_ItemName)
        {
            Txt_ItemName->SetText(FText::FromString(TEXT("Preview Item")));
            Txt_ItemName->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
    }
    else
    {
        // Show empty slot icon
        if (IMG_Icon)
        {
            SetIconBrush(EmptySlotIcon);
        }

        // Hide item name
        if (Txt_ItemName)
        {
            Txt_ItemName->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    ApplyRarityBorder();
    UpdateQuantityDisplay();
}

void URInventorySlotWidget::ApplyRarityBorder()
{
    if (!RarityBorder)
    {
        return;
    }

    // Selected state takes priority
    if (bIsSelected && SelectedBorderBrush.GetResourceObject())
    {
        RarityBorder->SetBrush(SelectedBorderBrush);
        return;
    }

    if (CurrentEntry.IsValidEntry() && CurrentEntry.ItemDefinition)
    {
        ERItemRarity Rarity = CurrentEntry.ItemDefinition->Visual.DefaultRarity;
        if (const FSlateBrush* RarityBrush = RarityBorderBrushes.Find(Rarity))
        {
            RarityBorder->SetBrush(*RarityBrush);
        }
        else
        {
            RarityBorder->SetBrush(DefaultBorderBrush);
        }
    }
    else if (bIsEditorPreview)
    {
        // Use a default rarity for preview (e.g., Common or Rare for visibility)
        if (const FSlateBrush* RarityBrush = RarityBorderBrushes.Find(ERItemRarity::Rare))
        {
            RarityBorder->SetBrush(*RarityBrush);
        }
        else
        {
            RarityBorder->SetBrush(DefaultBorderBrush);
        }
    }
    else
    {
        RarityBorder->SetBrush(DefaultBorderBrush);
    }
}

void URInventorySlotWidget::UpdateQuantityDisplay()
{
    if (!Txt_Quantity)
    {
        return;
    }

    int32 DisplayQuantity = 0;

    if (CurrentEntry.IsValidEntry())
    {
        DisplayQuantity = CurrentEntry.Quantity;
    }
    else if (bIsEditorPreview)
    {
        DisplayQuantity = PreviewQuantity;
    }

    if (DisplayQuantity >= MinQuantityToShow)
    {
        Txt_Quantity->SetText(FText::AsNumber(DisplayQuantity));
        Txt_Quantity->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else
    {
        Txt_Quantity->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void URInventorySlotWidget::HandleButtonClicked()
{
    OnSlotClicked.Broadcast(this, LastPointerEvent);
}

FReply URInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

    LastPointerEvent = InMouseEvent;

    // Allow drag detection for non-empty slots (but not editor preview)
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && CurrentEntry.IsValidEntry())
    {
        return Reply.DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
    }

    return Reply;
}

FReply URInventorySlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    FReply Reply = Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

    // Broadcast slot clicked event (if no button widget to handle it)
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && !Btn_Slot)
    {
        OnSlotClicked.Broadcast(this, InMouseEvent);
    }

    return Reply;
}

void URInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    if (!CurrentEntry.IsValidEntry())
    {
        return;
    }

    // Create drag drop operation
    UDragDropOperation* DragOp = NewObject<UDragDropOperation>();
    DragOp->Payload = CurrentEntry.ItemDefinition;
    DragOp->DefaultDragVisual = this;
    DragOp->Pivot = EDragPivot::CenterCenter;

    // Store slot index in tag for identification
    DragOp->Tag = FString::FromInt(SlotIndex);

    OutOperation = DragOp;
    OnDragStarted.Broadcast(this);
}

bool URInventorySlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

    // Accept any item drag
    if (InOperation && InOperation->Payload)
    {
        if (Cast<URItemDefinition>(InOperation->Payload))
        {
            return true;
        }
    }

    return false;
}

bool URInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

    // Handle drop - this will be processed by the parent grid widget
    // which has access to the inventory component
    return true;
}
