// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "REffectIconWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/WrapBox.h"
#include "UI/Models/RCombatUIModels.h"
#include "RPortraitWidget.generated.h"

/**
 * 
 */
UCLASS()
class RIZZGAME_API URPortraitWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TB_Health;

	UPROPERTY(meta = (BindWidget))
	USizeBox* SB_HealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_Health;

	UPROPERTY(meta = (BindWidget))
	USizeBox* SB_KineticShieldBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_KineticShield;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TB_KineticShield;

	UPROPERTY(meta = (BindWidget))
	USizeBox* SB_EnergyShieldBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_EnergyShield;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TB_EnergyShield;

	UPROPERTY(meta = (BindWidget))
	UOverlay* EffectsOverlay;

	UPROPERTY(meta = (BindWidget))
	UWrapBox* WB_StatusEffects;

	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Portrait;

	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Border;

	UPROPERTY(EditAnywhere, Category="Portrait")
	FLinearColor PlayerColor = FLinearColor(0.1f, 0.6f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Portrait")
	FLinearColor EnemyColor = FLinearColor(0.9f, 0.15f, 0.15f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Portrait")
	FLinearColor NeutralColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Portrait")
	FLinearColor KineticShieldColor = FLinearColor(0.3f, 0.6f, 0.9f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Portrait")
	FLinearColor EnergyShieldColor = FLinearColor(0.9f, 0.7f, 0.1f, 1.0f);

	/** Whether to show the status effects panel. */
	UPROPERTY(EditAnywhere, Category="Rizz")
	bool bShowStatusEffects = true;

	/** Whether to show stat text labels (health, shields). */
	UPROPERTY(EditAnywhere, Category="Rizz")
	bool bShowStatTexts = true;

	/** Override progress bar height in pixels. 0 means use the default size from UMG. */
	UPROPERTY(EditAnywhere, Category="Rizz")
	float BarHeightOverride = 0.f;

	UPROPERTY(EditAnywhere, Category="Preview")
	FPortraitViewModel PreviewData;

	UPROPERTY(EditAnywhere)
	int32 MaxVisibleStatusEffects = 5;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UREffectIconWidget> EffectIconWidgetClass;

public:
	void Init(FPortraitViewModel& ViewModel);

protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
};
