// RDefaultButton.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "RDefaultButton.generated.h"

class UImage;
class USoundBase;
class UWidgetAnimation;

UENUM(BlueprintType)
enum class EDefaultButtonState : uint8
{
	Normal,
	Hovered,
	Pressed,
	Disabled
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRDefaultButtonSimpleEvent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRDefaultButtonClicked, const FPointerEvent&, PointerEvent);

UCLASS()
class RIZZGAME_API URDefaultButton : public UUserWidget
{
	GENERATED_BODY()

public: /* ───────── PUBLIC ───────── */

	URDefaultButton(const FObjectInitializer& ObjectInitializer);

	// Settable (Design/Runtime)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RDefaultButton|Input")
	bool bToggleable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RDefaultButton|Input")
	bool bFocusableButton = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RDefaultButton|Input")
	TArray<FKey> ConfirmKeys = {EKeys::Enter, EKeys::SpaceBar, EKeys::Virtual_Accept, EKeys::Gamepad_FaceButton_Bottom};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RDefaultButton|SFX")
	USoundBase* HoverSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RDefaultButton|SFX")
	USoundBase* ClickSound = nullptr;

	/** Tek ikon (her state'de aynı görsel), tint ile renklendiriyoruz */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RDefaultButton|Content", meta=(ExposeOnSpawn="true"))
	FSlateBrush IconBrush;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RDefaultButton|Content")
	FLinearColor IconTint_Normal = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RDefaultButton|Content")
	FLinearColor IconTint_Hovered = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RDefaultButton|Content")
	FLinearColor IconTint_Pressed = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RDefaultButton|Content")
	FLinearColor IconTint_Disabled = FLinearColor(1, 1, 1, 0.5f);

	/** Tek border: state’e göre bu brush’lardan biri uygulanacak */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RDefaultButton|Content", meta=(ExposeOnSpawn="true"))
	FSlateBrush BackgroundBrush_Normal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RDefaultButton|Content", meta=(ExposeOnSpawn="true"))
	FSlateBrush BackgroundBrush_Hovered;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RDefaultButton|Content", meta=(ExposeOnSpawn="true"))
	FSlateBrush BackgroundBrush_Pressed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RDefaultButton|Content", meta=(ExposeOnSpawn="true"))
	FSlateBrush BackgroundBrush_Disabled;

	UFUNCTION(BlueprintCallable, Category="RDefaultButton|State")
	void SetEnabled(bool bInEnabled);

	UFUNCTION(BlueprintCallable, Category="RDefaultButton|Toggle")
	void SetToggled(bool bInToggled, bool bBroadcast = true);

	UFUNCTION(BlueprintCallable, Category="RDefaultButton|Input")
	void SimulateClick();

	UFUNCTION(BlueprintCallable, Category="RDefaultButton|Content")
	void SetIconBrush(const FSlateBrush& NewBrush);

	UFUNCTION(BlueprintCallable, Category="RDefaultButton|Content")
	void SetIconTints(FLinearColor Normal, FLinearColor Hovered, FLinearColor Pressed, FLinearColor Disabled);

	UFUNCTION(BlueprintCallable, Category="RDefaultButton|Content")
	void SetStyle(const FSlateBrush& InIcon,
	              const FSlateBrush& InBgNormal,
	              const FSlateBrush& InBgHover,
	              const FSlateBrush& InBgPressed,
	              const FSlateBrush& InBgDisabled,
	              FLinearColor InTintNormal = FLinearColor::White,
	              FLinearColor InTintHover = FLinearColor::White,
	              FLinearColor InTintPressed = FLinearColor::White,
	              FLinearColor InTintDisabled = FLinearColor(1, 1, 1, 0.5f));

	// Readable
	UFUNCTION(BlueprintPure, Category="RDefaultButton|State")
	bool IsEnabledForInteraction() const { return bEnabled; }

	UFUNCTION(BlueprintPure, Category="RDefaultButton|Toggle")
	bool IsToggled() const { return bToggled; }

	UPROPERTY(BlueprintAssignable, Category="RDefaultButton|Events")
	FRDefaultButtonSimpleEvent OnHovered;
	UPROPERTY(BlueprintAssignable, Category="RDefaultButton|Events")
	FRDefaultButtonSimpleEvent OnUnhovered;
	UPROPERTY(BlueprintAssignable, Category="RDefaultButton|Events")
	FRDefaultButtonSimpleEvent OnPressedSimple;
	UPROPERTY(BlueprintAssignable, Category="RDefaultButton|Events")
	FRDefaultButtonSimpleEvent OnReleased;
	UPROPERTY(BlueprintAssignable, Category="RDefaultButton|Events")
	FRDefaultButtonClicked OnClicked;

	/** Bind’lar — sadece tek border ve tek icon */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="RDefaultButton|Bind")
	UImage* IMG_Icon = nullptr;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="RDefaultButton|Bind")
	UBorder* B_Background = nullptr;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="RDefaultButton|Bind")
	UButton* B_Button = nullptr;

protected: /* ─────── PROTECTED ─────── */

	// UUserWidget
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Mouse input
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	void UpdateVisuals();
	void PlayAnimIfAvailable(UWidgetAnimation* Anim, float PlayRate = 1.f,
	                         EUMGSequencePlayMode::Type PlayMode = EUMGSequencePlayMode::Forward);
	void SetState(EDefaultButtonState NewState);
	void ApplyBrushesFromProperties();
	void Play2DSound(USoundBase* SFX) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="RDefaultButton|State")
	EDefaultButtonState State = EDefaultButtonState::Normal;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="RDefaultButton|State")
	bool bEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="RDefaultButton|State")
	bool bHovered = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="RDefaultButton|State")
	bool bPressed = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="RDefaultButton|State")
	bool bToggled = false;

private: /* ─────── PRIVATE ─────── */
	static void SetVis(UWidget* W, ESlateVisibility Vis);
	static void SetBrushOn(UWidget* Widget, const FSlateBrush& Brush);
	static bool BrushHasResource(const FSlateBrush& Brush);

	/** O anki state için hangi brush kullanılacak? */
	const FSlateBrush* ResolveBackgroundBrushForState() const;
};
