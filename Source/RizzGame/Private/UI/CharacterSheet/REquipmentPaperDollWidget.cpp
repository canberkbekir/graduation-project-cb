// REquipmentPaperDollWidget.cpp

#include "UI/CharacterSheet/REquipmentPaperDollWidget.h"
#include "UI/CharacterSheet/REquipmentSlotWidget.h"
#include "UI/CharacterSheet/RCharacterSheetActor.h"
#include "Core/RCharacterBase.h"
#include "Components/REquipmentComponent.h"
#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"

void UREquipmentPaperDollWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    // Show editor preview when designing
    if (IsDesignTime() && bShowEditorPreview)
    {
        CreateEditorPreview();
    }
}

void UREquipmentPaperDollWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UREquipmentPaperDollWidget::CreateEditorPreview()
{
    // Gather all slot widgets
    TArray<UREquipmentSlotWidget*> AllSlots;
    if (Slot_Head) AllSlots.Add(Slot_Head);
    if (Slot_Chest) AllSlots.Add(Slot_Chest);
    if (Slot_Hands) AllSlots.Add(Slot_Hands);
    if (Slot_Feet) AllSlots.Add(Slot_Feet);
    if (Slot_MainHand) AllSlots.Add(Slot_MainHand);
    if (Slot_OffHand) AllSlots.Add(Slot_OffHand);

    // Set up preview state on each slot
    for (int32 i = 0; i < AllSlots.Num(); ++i)
    {
        if (i < EditorPreviewFilledSlots)
        {
            AllSlots[i]->SetupAsEditorPreview();
        }
        else
        {
            AllSlots[i]->ClearSlot();
        }
    }
}

void UREquipmentPaperDollWidget::NativeDestruct()
{
    UnbindEquipmentSlots();
    Super::NativeDestruct();
}

void UREquipmentPaperDollWidget::Init(ARCharacterBase* InCharacter, ARCharacterSheetActor* InPreviewActor)
{
    // Unbind previous
    UnbindEquipmentSlots();

    Character = InCharacter;
    PreviewActor = InPreviewActor;

    if (Character)
    {
        EquipmentComponent = Character->GetEquipmentComponent();
    }
    else
    {
        EquipmentComponent = nullptr;
    }

    // Set up the preview actor with character visuals
    if (PreviewActor && Character)
    {
        PreviewActor->ApplyFromCharacter(Character);
    }

    // Set up render target
    SetupRenderTarget();

    // Bind equipment slots
    BindEquipmentSlots();

    // Initial refresh
    Refresh();
}

void UREquipmentPaperDollWidget::Refresh()
{
    // Slots auto-update via their equipment component binding
    // This method is here for manual refresh if needed
}

void UREquipmentPaperDollWidget::BindEquipmentSlots()
{
    if (!EquipmentComponent)
    {
        return;
    }

    auto BindSlot = [this](UREquipmentSlotWidget* SlotWidget, EREquipmentSlot InSlotType)
    {
        if (SlotWidget)
        {
            SlotWidget->SlotType = InSlotType;
            SlotWidget->BindToEquipmentComponent(EquipmentComponent);
        }
    };

    BindSlot(Slot_Head, EREquipmentSlot::Head);
    BindSlot(Slot_Chest, EREquipmentSlot::Chest);
    BindSlot(Slot_Hands, EREquipmentSlot::Hands);
    BindSlot(Slot_Feet, EREquipmentSlot::Feet);
    BindSlot(Slot_MainHand, EREquipmentSlot::MainHand);
    BindSlot(Slot_OffHand, EREquipmentSlot::OffHand);
}

void UREquipmentPaperDollWidget::UnbindEquipmentSlots()
{
    auto UnbindSlot = [](UREquipmentSlotWidget* SlotWidget)
    {
        if (SlotWidget)
        {
            SlotWidget->UnbindFromEquipmentComponent();
        }
    };

    UnbindSlot(Slot_Head);
    UnbindSlot(Slot_Chest);
    UnbindSlot(Slot_Hands);
    UnbindSlot(Slot_Feet);
    UnbindSlot(Slot_MainHand);
    UnbindSlot(Slot_OffHand);
}

void UREquipmentPaperDollWidget::SetupRenderTarget()
{
    if (!Img_CharacterPreview || !PreviewActor)
    {
        return;
    }

    // Get the render target from the preview actor
    // The preview actor should have a public getter or we access via reflection
    // For now, we'll assume the render target is set up in Blueprint
}

UREquipmentSlotWidget* UREquipmentPaperDollWidget::GetEquipmentSlot(EREquipmentSlot SlotType) const
{
    switch (SlotType)
    {
    case EREquipmentSlot::Head:
        return Slot_Head;
    case EREquipmentSlot::Chest:
        return Slot_Chest;
    case EREquipmentSlot::Hands:
        return Slot_Hands;
    case EREquipmentSlot::Feet:
        return Slot_Feet;
    case EREquipmentSlot::MainHand:
        return Slot_MainHand;
    case EREquipmentSlot::OffHand:
        return Slot_OffHand;
    default:
        return nullptr;
    }
}

void UREquipmentPaperDollWidget::SetPreviewRotation(float YawDegrees)
{
    if (PreviewActor)
    {
        PreviewActor->SetPreviewYaw(YawDegrees);
    }
}

void UREquipmentPaperDollWidget::AddPreviewRotation(float DeltaYaw)
{
    if (PreviewActor)
    {
        PreviewActor->AddPreviewYaw(DeltaYaw);
    }
}

void UREquipmentPaperDollWidget::ResetPreviewRotation()
{
    SetPreviewRotation(DefaultRotation);
}

FReply UREquipmentPaperDollWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bEnableDragRotation && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        // Check if click is on the preview image area
        if (Img_CharacterPreview)
        {
            FGeometry ImageGeometry = Img_CharacterPreview->GetCachedGeometry();
            FVector2D LocalPos = ImageGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
            FVector2D ImageSize = ImageGeometry.GetLocalSize();

            if (LocalPos.X >= 0 && LocalPos.X <= ImageSize.X && LocalPos.Y >= 0 && LocalPos.Y <= ImageSize.Y)
            {
                bIsDragging = true;
                LastMousePosition = InMouseEvent.GetScreenSpacePosition();
                return FReply::Handled().CaptureMouse(TakeWidget());
            }
        }
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UREquipmentPaperDollWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bIsDragging && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        bIsDragging = false;
        return FReply::Handled().ReleaseMouseCapture();
    }

    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UREquipmentPaperDollWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bIsDragging && bEnableDragRotation)
    {
        FVector2D CurrentMousePosition = InMouseEvent.GetScreenSpacePosition();
        float DeltaX = CurrentMousePosition.X - LastMousePosition.X;

        AddPreviewRotation(DeltaX * RotationSensitivity);

        LastMousePosition = CurrentMousePosition;

        return FReply::Handled();
    }

    return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}
