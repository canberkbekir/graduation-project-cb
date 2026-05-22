// REquipmentSlotWidget.cpp

#include "UI/CharacterSheet/REquipmentSlotWidget.h"
#include "Components/REquipmentComponent.h"
#include "Items/RItemDefinition.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

UREquipmentSlotWidget::UREquipmentSlotWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UREquipmentSlotWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    // Apply slot size
    ApplySlotSize();

    // Show editor preview when designing
    if (IsDesignTime() && bShowEditorPreview && bEditorPreviewFilled)
    {
        SetupAsEditorPreview();
    }
    else
    {
        UpdateVisualState();
    }
}

void UREquipmentSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Apply slot size
    ApplySlotSize();

    // Bind button click if available
    if (Btn_Slot)
    {
        Btn_Slot->OnClicked.AddDynamic(this, &UREquipmentSlotWidget::HandleButtonClicked);
    }

    UpdateVisualState();
}

void UREquipmentSlotWidget::ApplySlotSize()
{
    if (SlotSize > 0 && SizeBoxRoot)
    {
        SizeBoxRoot->SetWidthOverride(SlotSize);
        SizeBoxRoot->SetHeightOverride(SlotSize);
    }
}

void UREquipmentSlotWidget::NativeDestruct()
{
    if (Btn_Slot)
    {
        Btn_Slot->OnClicked.RemoveDynamic(this, &UREquipmentSlotWidget::HandleButtonClicked);
    }

    UnbindFromEquipmentComponent();
    Super::NativeDestruct();
}

void UREquipmentSlotWidget::BindToEquipmentComponent(UREquipmentComponent* InEquipmentComponent)
{
    // Unbind from previous if any
    UnbindFromEquipmentComponent();

    BoundEquipmentComponent = InEquipmentComponent;

    if (BoundEquipmentComponent)
    {
        BoundEquipmentComponent->OnEquipmentChanged.AddDynamic(this, &UREquipmentSlotWidget::HandleEquipmentChanged);

        // Initialize with current state
        URItemDefinition* CurrentEquipped = BoundEquipmentComponent->GetEquippedItem(SlotType);
        SetItem(CurrentEquipped);
    }
}

void UREquipmentSlotWidget::UnbindFromEquipmentComponent()
{
    if (BoundEquipmentComponent)
    {
        BoundEquipmentComponent->OnEquipmentChanged.RemoveDynamic(this, &UREquipmentSlotWidget::HandleEquipmentChanged);
        BoundEquipmentComponent = nullptr;
    }
}

void UREquipmentSlotWidget::SetItem(URItemDefinition* InItemDef)
{
    CurrentItemDef = InItemDef;
    bIsEditorPreview = false;
    UpdateVisualState();
}

void UREquipmentSlotWidget::ClearSlot()
{
    CurrentItemDef = nullptr;
    bIsEditorPreview = false;
    UpdateVisualState();
}

void UREquipmentSlotWidget::SetupAsEditorPreview()
{
    CurrentItemDef = nullptr;
    bIsEditorPreview = true;

    // Show preview icon
    if (IMG_Icon)
    {
        if (PreviewItemIcon.GetResourceObject())
        {
            SetIconBrush(PreviewItemIcon);
        }
        else
        {
            SetIconBrush(EmptySlotIcon);
        }
    }

    // Hide slot label during preview
    if (Txt_SlotLabel)
    {
        Txt_SlotLabel->SetVisibility(ESlateVisibility::Collapsed);
    }

    // Show a preview rarity border (e.g., Rare for visibility)
    if (RarityBorder)
    {
        if (const FSlateBrush* RarityBrush = RarityBorderBrushes.Find(ERItemRarity::Rare))
        {
            RarityBorder->SetBrush(*RarityBrush);
        }
        else
        {
            RarityBorder->SetBrush(DefaultBorderBrush);
        }
    }
}

void UREquipmentSlotWidget::SetToggled(bool bInToggled, bool bBroadcast)
{
    if (bIsToggled == bInToggled)
    {
        return;
    }

    bIsToggled = bInToggled;
    ApplyRarityBorder();
}

void UREquipmentSlotWidget::HandleEquipmentChanged(EREquipmentSlot ChangedSlot, URItemDefinition* NewItem)
{
    if (ChangedSlot == SlotType)
    {
        SetItem(NewItem);
    }
}

void UREquipmentSlotWidget::HandleButtonClicked()
{
    OnClicked.Broadcast(LastPointerEvent);
}

void UREquipmentSlotWidget::SetIconBrush(const FSlateBrush& NewBrush)
{
    if (IMG_Icon)
    {
        IMG_Icon->SetBrush(NewBrush);
    }
}

void UREquipmentSlotWidget::UpdateVisualState()
{
    if (CurrentItemDef)
    {
        // Show item icon
        if (IMG_Icon)
        {
            // Load the icon asynchronously if needed
            if (UTexture2D* Icon = CurrentItemDef->Visual.Icon.LoadSynchronous())
            {
                FSlateBrush ItemBrush;
                ItemBrush.SetResourceObject(Icon);
                ItemBrush.ImageSize = FVector2D(64.0f, 64.0f);
                SetIconBrush(ItemBrush);
            }
        }

        // Hide slot label
        if (Txt_SlotLabel)
        {
            Txt_SlotLabel->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
    else if (bIsEditorPreview)
    {
        // Already handled in SetupAsEditorPreview
    }
    else
    {
        // Show empty slot icon
        if (IMG_Icon)
        {
            SetIconBrush(EmptySlotIcon);
        }

        // Show slot label
        if (Txt_SlotLabel)
        {
            Txt_SlotLabel->SetText(GetSlotDisplayName());
            Txt_SlotLabel->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
    }

    ApplyRarityBorder();
}

void UREquipmentSlotWidget::ApplyRarityBorder()
{
    if (!RarityBorder)
    {
        return;
    }

    // Toggled/selected state takes priority
    if (bIsToggled && SelectedBorderBrush.GetResourceObject())
    {
        RarityBorder->SetBrush(SelectedBorderBrush);
        return;
    }

    if (CurrentItemDef)
    {
        ERItemRarity Rarity = CurrentItemDef->Visual.DefaultRarity;
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
        // Keep preview border (set in SetupAsEditorPreview)
    }
    else
    {
        RarityBorder->SetBrush(DefaultBorderBrush);
    }
}

FText UREquipmentSlotWidget::GetSlotDisplayName() const
{
    switch (SlotType)
    {
    case EREquipmentSlot::Head:
        return FText::FromString(TEXT("Head"));
    case EREquipmentSlot::Chest:
        return FText::FromString(TEXT("Chest"));
    case EREquipmentSlot::Hands:
        return FText::FromString(TEXT("Hands"));
    case EREquipmentSlot::Feet:
        return FText::FromString(TEXT("Feet"));
    case EREquipmentSlot::MainHand:
        return FText::FromString(TEXT("Main Hand"));
    case EREquipmentSlot::OffHand:
        return FText::FromString(TEXT("Off Hand"));
    case EREquipmentSlot::Accessory1:
        return FText::FromString(TEXT("Accessory"));
    case EREquipmentSlot::Accessory2:
        return FText::FromString(TEXT("Accessory"));
    default:
        return FText::FromString(TEXT("Slot"));
    }
}

FReply UREquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

    LastPointerEvent = InMouseEvent;

    // Allow drag detection
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && CurrentItemDef)
    {
        return Reply.DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
    }

    return Reply;
}

FReply UREquipmentSlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    FReply Reply = Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

    // Broadcast clicked event (if no button widget to handle it)
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && !Btn_Slot)
    {
        OnClicked.Broadcast(InMouseEvent);
    }

    return Reply;
}

void UREquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    if (!CurrentItemDef)
    {
        return;
    }

    // Create drag drop operation
    UDragDropOperation* DragOp = NewObject<UDragDropOperation>();
    DragOp->Payload = CurrentItemDef;
    DragOp->DefaultDragVisual = this;
    DragOp->Pivot = EDragPivot::CenterCenter;

    OutOperation = DragOp;
}

bool UREquipmentSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

    // Accept items that can be equipped in this slot
    if (InOperation && InOperation->Payload)
    {
        if (URItemDefinition* DraggedItem = Cast<URItemDefinition>(InOperation->Payload))
        {
            // For now, accept any item - you can add slot validation here
            return true;
        }
    }

    return false;
}

bool UREquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

    if (!InOperation || !InOperation->Payload || !BoundEquipmentComponent)
    {
        return false;
    }

    if (URItemDefinition* DroppedItem = Cast<URItemDefinition>(InOperation->Payload))
    {
        // Equip the dropped item
        return BoundEquipmentComponent->EquipItem(DroppedItem, SlotType, true);
    }

    return false;
}
