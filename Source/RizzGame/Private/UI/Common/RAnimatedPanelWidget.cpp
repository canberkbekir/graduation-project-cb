// RAnimatedPanelWidget.cpp

#include "UI/Common/RAnimatedPanelWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

URAnimatedPanelWidget::URAnimatedPanelWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void URAnimatedPanelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Store original values
    OriginalOpacity = GetRenderOpacity();
    OriginalScale = GetRenderTransform().Scale;
}

void URAnimatedPanelWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsAnimating)
    {
        UpdateAnimation(InDeltaTime);
    }
}

void URAnimatedPanelWidget::PlayOpenAnimation()
{
    // Check for UMG animation override
    if (OpenAnimation)
    {
        BP_OnOpenAnimationStart();
        PlayAnimation(OpenAnimation);

        // Set up timer to call completion
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            FinishOpenAnimation();
        }, OpenAnimation->GetEndTime(), false);

        return;
    }

    if (OpenAnimationType == ERPanelAnimationType::None)
    {
        bIsOpen = true;
        SetRenderOpacity(1.0f);
        SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
        SetRenderTranslation(FVector2D::ZeroVector);
        SetRenderScale(FVector2D(1.0f, 1.0f));
        return;
    }

    BP_OnOpenAnimationStart();
    SetupForOpenAnimation();

    bIsAnimating = true;
    bIsOpening = true;
    AnimationProgress = 0.0f;
}

void URAnimatedPanelWidget::PlayCloseAnimation()
{
    // Check for UMG animation override
    if (CloseAnimation)
    {
        BP_OnCloseAnimationStart();
        PlayAnimation(CloseAnimation);

        // Set up timer to call completion
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            FinishCloseAnimation();
        }, CloseAnimation->GetEndTime(), false);

        return;
    }

    if (CloseAnimationType == ERPanelAnimationType::None)
    {
        bIsOpen = false;
        return;
    }

    BP_OnCloseAnimationStart();
    SetupForCloseAnimation();

    bIsAnimating = true;
    bIsOpening = false;
    AnimationProgress = 0.0f;
}

void URAnimatedPanelWidget::SetupForOpenAnimation()
{
    SetRenderTransformPivot(FVector2D(0.5f, 0.5f));

    // Set initial state based on animation type
    switch (OpenAnimationType)
    {
    case ERPanelAnimationType::SlideFromLeft:
        SetRenderTranslation(FVector2D(-500.0f, 0.0f));
        break;
    case ERPanelAnimationType::SlideFromRight:
        SetRenderTranslation(FVector2D(500.0f, 0.0f));
        break;
    case ERPanelAnimationType::SlideFromTop:
        SetRenderTranslation(FVector2D(0.0f, -500.0f));
        break;
    case ERPanelAnimationType::SlideFromBottom:
        SetRenderTranslation(FVector2D(0.0f, 500.0f));
        break;
    case ERPanelAnimationType::FadeIn:
        SetRenderOpacity(0.0f);
        break;
    case ERPanelAnimationType::ScaleIn:
        SetRenderScale(FVector2D(0.0f, 0.0f));
        break;
    default:
        break;
    }
}

void URAnimatedPanelWidget::SetupForCloseAnimation()
{
    SetRenderTransformPivot(FVector2D(0.5f, 0.5f));

    // Set to fully open state
    SetRenderTranslation(FVector2D::ZeroVector);
    SetRenderOpacity(1.0f);
    SetRenderScale(FVector2D(1.0f, 1.0f));
}

void URAnimatedPanelWidget::UpdateAnimation(float DeltaTime)
{
    if (AnimationDuration <= 0.0f)
    {
        AnimationProgress = 1.0f;
    }
    else
    {
        AnimationProgress += DeltaTime / AnimationDuration;
    }

    AnimationProgress = FMath::Clamp(AnimationProgress, 0.0f, 1.0f);

    // Apply easing (ease out cubic)
    float EasedProgress = 1.0f - FMath::Pow(1.0f - AnimationProgress, 3.0f);

    ERPanelAnimationType CurrentAnimType = bIsOpening ? OpenAnimationType : CloseAnimationType;
    ApplyAnimationProgress(EasedProgress, CurrentAnimType, bIsOpening);

    if (AnimationProgress >= 1.0f)
    {
        bIsAnimating = false;

        if (bIsOpening)
        {
            FinishOpenAnimation();
        }
        else
        {
            FinishCloseAnimation();
        }
    }
}

void URAnimatedPanelWidget::ApplyAnimationProgress(float Progress, ERPanelAnimationType AnimType, bool bIsOpeningAnim)
{
    // For close animations, we want to reverse the progress
    float EffectiveProgress = bIsOpeningAnim ? Progress : (1.0f - Progress);

    switch (AnimType)
    {
    case ERPanelAnimationType::SlideFromLeft:
        SetRenderTranslation(FVector2D(FMath::Lerp(-500.0f, 0.0f, EffectiveProgress), 0.0f));
        break;
    case ERPanelAnimationType::SlideFromRight:
        SetRenderTranslation(FVector2D(FMath::Lerp(500.0f, 0.0f, EffectiveProgress), 0.0f));
        break;
    case ERPanelAnimationType::SlideFromTop:
        SetRenderTranslation(FVector2D(0.0f, FMath::Lerp(-500.0f, 0.0f, EffectiveProgress)));
        break;
    case ERPanelAnimationType::SlideFromBottom:
        SetRenderTranslation(FVector2D(0.0f, FMath::Lerp(500.0f, 0.0f, EffectiveProgress)));
        break;
    case ERPanelAnimationType::FadeIn:
        SetRenderOpacity(FMath::Lerp(0.0f, 1.0f, EffectiveProgress));
        break;
    case ERPanelAnimationType::ScaleIn:
        {
            float Scale = FMath::Lerp(0.0f, 1.0f, EffectiveProgress);
            SetRenderScale(FVector2D(Scale, Scale));
        }
        break;
    default:
        break;
    }
}

void URAnimatedPanelWidget::FinishOpenAnimation()
{
    bIsOpen = true;
    bIsAnimating = false;

    // Reset to normal state
    SetRenderTranslation(FVector2D::ZeroVector);
    SetRenderOpacity(1.0f);
    SetRenderScale(FVector2D(1.0f, 1.0f));

    BP_OnOpenAnimationComplete();
    OnOpenAnimationComplete.Broadcast();
}

void URAnimatedPanelWidget::FinishCloseAnimation()
{
    bIsOpen = false;
    bIsAnimating = false;

    BP_OnCloseAnimationComplete();
    OnCloseAnimationComplete.Broadcast();
}

FVector2D URAnimatedPanelWidget::GetAnimationOffset(ERPanelAnimationType AnimType, float Progress) const
{
    const float OffscreenOffset = 500.0f;

    switch (AnimType)
    {
    case ERPanelAnimationType::SlideFromLeft:
        return FVector2D(-OffscreenOffset * (1.0f - Progress), 0.0f);
    case ERPanelAnimationType::SlideFromRight:
        return FVector2D(OffscreenOffset * (1.0f - Progress), 0.0f);
    case ERPanelAnimationType::SlideFromTop:
        return FVector2D(0.0f, -OffscreenOffset * (1.0f - Progress));
    case ERPanelAnimationType::SlideFromBottom:
        return FVector2D(0.0f, OffscreenOffset * (1.0f - Progress));
    default:
        return FVector2D::ZeroVector;
    }
}
