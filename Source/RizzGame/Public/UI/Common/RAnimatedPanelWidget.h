// RAnimatedPanelWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RAnimatedPanelWidget.generated.h"

class UWidgetAnimation;

/** Animation types for panel transitions. */
UENUM(BlueprintType)
enum class ERPanelAnimationType : uint8
{
    None        UMETA(DisplayName = "None"),
    SlideFromLeft   UMETA(DisplayName = "Slide From Left"),
    SlideFromRight  UMETA(DisplayName = "Slide From Right"),
    SlideFromTop    UMETA(DisplayName = "Slide From Top"),
    SlideFromBottom UMETA(DisplayName = "Slide From Bottom"),
    FadeIn      UMETA(DisplayName = "Fade In"),
    ScaleIn     UMETA(DisplayName = "Scale In")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPanelAnimationComplete);

/**
 * Base class for animated panel widgets.
 * Provides built-in support for common panel animations like slide, fade, and scale.
 */
UCLASS()
class RIZZGAME_API URAnimatedPanelWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    URAnimatedPanelWidget(const FObjectInitializer& ObjectInitializer);

    /* ───────── Animation Settings ───────── */

    /** Animation type for opening the panel. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    ERPanelAnimationType OpenAnimationType = ERPanelAnimationType::SlideFromLeft;

    /** Animation type for closing the panel. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    ERPanelAnimationType CloseAnimationType = ERPanelAnimationType::SlideFromLeft;

    /** Duration of animations in seconds. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float AnimationDuration = 0.3f;

    /** Optional UMG animation for open (override programmatic animation). */
    UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetAnimOptional), Category = "Animation")
    UWidgetAnimation* OpenAnimation = nullptr;

    /** Optional UMG animation for close (override programmatic animation). */
    UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetAnimOptional), Category = "Animation")
    UWidgetAnimation* CloseAnimation = nullptr;

    /* ───────── Animation API ───────── */

    /** Play the open animation. */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayOpenAnimation();

    /** Play the close animation. */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayCloseAnimation();

    /** Check if an animation is currently playing. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation")
    bool IsAnimating() const { return bIsAnimating; }

    /** Check if the panel is fully open (animation complete). */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation")
    bool IsOpen() const { return bIsOpen; }

    /* ───────── Events ───────── */

    UPROPERTY(BlueprintAssignable, Category = "Animation|Events")
    FOnPanelAnimationComplete OnOpenAnimationComplete;

    UPROPERTY(BlueprintAssignable, Category = "Animation|Events")
    FOnPanelAnimationComplete OnCloseAnimationComplete;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /** Blueprint hook called when open animation starts. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void BP_OnOpenAnimationStart();

    /** Blueprint hook called when open animation completes. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void BP_OnOpenAnimationComplete();

    /** Blueprint hook called when close animation starts. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void BP_OnCloseAnimationStart();

    /** Blueprint hook called when close animation completes. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void BP_OnCloseAnimationComplete();

    /** Called to set up the initial state before open animation plays. */
    virtual void SetupForOpenAnimation();

    /** Called to set up the initial state before close animation plays. */
    virtual void SetupForCloseAnimation();

private:
    void UpdateAnimation(float DeltaTime);
    void ApplyAnimationProgress(float Progress, ERPanelAnimationType AnimType, bool bIsOpening);
    void FinishOpenAnimation();
    void FinishCloseAnimation();

    FVector2D GetAnimationOffset(ERPanelAnimationType AnimType, float Progress) const;

    UPROPERTY()
    bool bIsAnimating = false;

    UPROPERTY()
    bool bIsOpen = false;

    UPROPERTY()
    bool bIsOpening = false;

    UPROPERTY()
    float AnimationProgress = 0.0f;

    FVector2D OriginalPosition;
    FVector2D StartPosition;
    float OriginalOpacity = 1.0f;
    FVector2D OriginalScale = FVector2D(1.0f, 1.0f);
};
