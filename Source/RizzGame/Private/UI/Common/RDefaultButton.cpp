// RDefaultButton.cpp
 
#include "UI/Common/RDefaultButton.h"

#include "Components/Image.h"
#include "Components/Border.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

URDefaultButton::URDefaultButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

/* ───────── PUBLIC ───────── */

void URDefaultButton::SetEnabled(bool bInEnabled)
{
	if (bEnabled == bInEnabled)
	{
		return;
	}

	bEnabled = bInEnabled;
	if (!bEnabled)
	{
		SetState(EDefaultButtonState::Disabled);
	}
	else
	{
		SetState(bHovered ? EDefaultButtonState::Hovered : EDefaultButtonState::Normal);
	}
	UpdateVisuals();
}

void URDefaultButton::SetToggled(bool bInToggled, bool bBroadcast)
{
	if (!bToggleable)
	{
		bToggled = false;
		return;
	}
	if (bToggled == bInToggled)
	{
		return;
	}

	bToggled = bInToggled;
	UpdateVisuals();

	if (bBroadcast)
	{
		// Add OnToggled delegate later if needed
	}
}

void URDefaultButton::SimulateClick()
{
	if (!bEnabled)
	{
		return;
	}

	bPressed = true;
	SetState(EDefaultButtonState::Pressed);
	UpdateVisuals();

	bPressed = false;
	SetState(bHovered ? EDefaultButtonState::Hovered : EDefaultButtonState::Normal);
	UpdateVisuals();

	if (ClickSound)
	{
		Play2DSound(ClickSound);
	}

	FPointerEvent Dummy;
	OnClicked.Broadcast(Dummy);
	OnReleased.Broadcast();

	if (bToggleable)
	{
		SetToggled(!bToggled, /*bBroadcast*/true);
	}
}

void URDefaultButton::SetIconBrush(const FSlateBrush& NewBrush)
{
	IconBrush = NewBrush;
	if (IMG_Icon)
	{
		IMG_Icon->SetBrush(IconBrush);
	}
}

void URDefaultButton::SetIconTints(FLinearColor Normal, FLinearColor Hovered, FLinearColor Pressed,
                                   FLinearColor Disabled)
{
	IconTint_Normal = Normal;
	IconTint_Hovered = Hovered;
	IconTint_Pressed = Pressed;
	IconTint_Disabled = Disabled;
	UpdateVisuals();
}

void URDefaultButton::SetStyle(const FSlateBrush& InIcon,
                               const FSlateBrush& InBgNormal,
                               const FSlateBrush& InBgHover,
                               const FSlateBrush& InBgPressed,
                               const FSlateBrush& InBgDisabled,
                               FLinearColor InTintNormal,
                               FLinearColor InTintHover,
                               FLinearColor InTintPressed,
                               FLinearColor InTintDisabled)
{
	IconBrush = InIcon;
	BackgroundBrush_Normal = InBgNormal;
	BackgroundBrush_Hovered = InBgHover;
	BackgroundBrush_Pressed = InBgPressed;
	BackgroundBrush_Disabled = InBgDisabled;
	IconTint_Normal = InTintNormal;
	IconTint_Hovered = InTintHover;
	IconTint_Pressed = InTintPressed;
	IconTint_Disabled = InTintDisabled;

	ApplyBrushesFromProperties();
	UpdateVisuals();
}

/* ───────── PROTECTED ───────── */

void URDefaultButton::NativePreConstruct()
{
	Super::NativePreConstruct();
	ApplyBrushesFromProperties();
	UpdateVisuals();
}

void URDefaultButton::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Visible);
	if (bFocusableButton)
	{
		SetIsFocusable(true);
	}
}

void URDefaultButton::NativeDestruct()
{
	Super::NativeDestruct();
}

void URDefaultButton::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (!bEnabled)
	{
		return;
	}

	bHovered = true;
	SetState(EDefaultButtonState::Hovered);
	UpdateVisuals();
	if (HoverSound)
	{
		Play2DSound(HoverSound);
	}
	OnHovered.Broadcast();
}

void URDefaultButton::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (!bEnabled)
	{
		return;
	}

	bHovered = false;
	if (!bPressed)
	{
		SetState(EDefaultButtonState::Normal);
		UpdateVisuals();
	}
	OnUnhovered.Broadcast();
}

FReply URDefaultButton::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bEnabled)
	{
		return FReply::Unhandled();
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bPressed = true;
		SetState(EDefaultButtonState::Pressed);
		UpdateVisuals();
		OnPressedSimple.Broadcast();

		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply URDefaultButton::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bEnabled)
	{
		return FReply::Unhandled();
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		const bool bWasPressed = bPressed;
		bPressed = false;

		if (bWasPressed && IsHovered())
		{
			if (ClickSound)
			{
				Play2DSound(ClickSound);
			}
			OnClicked.Broadcast(InMouseEvent);

			if (bToggleable)
			{
				SetToggled(!bToggled, /*bBroadcast*/true);
			}
		}

		SetState(bHovered ? EDefaultButtonState::Hovered : EDefaultButtonState::Normal);
		UpdateVisuals();

		OnReleased.Broadcast();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void URDefaultButton::UpdateVisuals()
{
	// Tek border: state’e göre doğru brush’ı seçip uygula.
	if (B_Background)
	{
		if (const FSlateBrush* Bg = ResolveBackgroundBrushForState())
		{
			if (BrushHasResource(*Bg))
			{
				B_Background->SetBrush(*Bg);
			}
		}
		SetVis(B_Background, ESlateVisibility::SelfHitTestInvisible);
	}

	// Tek icon: her zaman açık, sadece tint state’e göre değişir.
	if (IMG_Icon)
	{
		SetVis(IMG_Icon, ESlateVisibility::SelfHitTestInvisible);

		const FLinearColor Tint =
			(State == EDefaultButtonState::Disabled)
				? IconTint_Disabled
				: (State == EDefaultButtonState::Pressed)
				? IconTint_Pressed
				: (State == EDefaultButtonState::Hovered)
				? IconTint_Hovered
				: IconTint_Normal;

		IMG_Icon->SetColorAndOpacity(Tint);
	}
}

void URDefaultButton::PlayAnimIfAvailable(UWidgetAnimation* Anim, float PlayRate, EUMGSequencePlayMode::Type PlayMode)
{
	if (Anim)
	{
		PlayAnimation(Anim, 0.f, 1, PlayMode, PlayRate);
	}
}

void URDefaultButton::SetState(EDefaultButtonState NewState)
{
	State = NewState;
}

void URDefaultButton::ApplyBrushesFromProperties()
{
	// İlk yüklemede ikon ve default background’ı uygula (Designer + runtime)
	if (IMG_Icon && IconBrush.GetResourceObject())
	{
		IMG_Icon->SetBrush(IconBrush);
	}
	if (B_Background && BackgroundBrush_Normal.GetResourceObject())
	{
		B_Background->SetBrush(BackgroundBrush_Normal);
	}
}

void URDefaultButton::Play2DSound(USoundBase* SFX) const
{
	if (!SFX)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::PlaySound2D(World, SFX);
	}
}

/* ───────── PRIVATE ───────── */

void URDefaultButton::SetVis(UWidget* W, ESlateVisibility Vis)
{
	if (W) { W->SetVisibility(Vis); }
}

void URDefaultButton::SetBrushOn(UWidget* Widget, const FSlateBrush& Brush)
{
	if (!Widget || !Brush.GetResourceObject())
	{
		return;
	}

	if (UImage* Img = Cast<UImage>(Widget))
	{
		Img->SetBrush(Brush);
		return;
	}
	if (UBorder* Border = Cast<UBorder>(Widget)) { Border->SetBrush(Brush); }
}

bool URDefaultButton::BrushHasResource(const FSlateBrush& Brush)
{
	return Brush.GetResourceObject() != nullptr;
}

const FSlateBrush* URDefaultButton::ResolveBackgroundBrushForState() const
{
	switch (State)
	{
	case EDefaultButtonState::Disabled: return &BackgroundBrush_Disabled;
	case EDefaultButtonState::Pressed: return &BackgroundBrush_Pressed;
	case EDefaultButtonState::Hovered: return &BackgroundBrush_Hovered;
	case EDefaultButtonState::Normal:
	default: return &BackgroundBrush_Normal;
	}
}
