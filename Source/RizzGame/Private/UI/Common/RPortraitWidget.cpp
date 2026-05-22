// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Common/RPortraitWidget.h"

void URPortraitWidget::NativeConstruct()
{
	Super::NativeConstruct();
	WB_StatusEffects->ClearChildren();
}

void URPortraitWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (IsDesignTime())
	{
		Init(PreviewData);
	}
}


void URPortraitWidget::Init(FPortraitViewModel& ViewModel)
{
	const ESlateVisibility TextVis = bShowStatTexts ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
	TB_Health->SetVisibility(TextVis);
	TB_Health->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), ViewModel.CurrentHealth, ViewModel.MaxHealth)));
	PB_Health->SetPercent(ViewModel.MaxHealth > 0 ? static_cast<float>(ViewModel.CurrentHealth) / ViewModel.MaxHealth : 0.f);
	IMG_Portrait->SetBrushFromTexture(ViewModel.PortraitTexture, false);

	if (BarHeightOverride > 0.f)
	{
		auto ApplyHeightOverride = [this](USizeBox* Box)
		{
			if (!Box) return;
			Box->SetHeightOverride(BarHeightOverride);
		};
		ApplyHeightOverride(SB_HealthBar);
		ApplyHeightOverride(SB_KineticShieldBar);
		ApplyHeightOverride(SB_EnergyShieldBar);
	}
	else
	{
		auto ClearHeightOverride = [](USizeBox* Box)
		{
			if (!Box) return;
			Box->ClearHeightOverride();
		};
		ClearHeightOverride(SB_HealthBar);
		ClearHeightOverride(SB_KineticShieldBar);
		ClearHeightOverride(SB_EnergyShieldBar);
	}

	// Kinetic shield
	{
		const bool bShowKinetic = ViewModel.MaxKineticShields > 0.f;
		const ESlateVisibility KineticVis = bShowKinetic ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		if (SB_KineticShieldBar)
			SB_KineticShieldBar->SetVisibility(KineticVis);
		if (PB_KineticShield)
			PB_KineticShield->SetVisibility(KineticVis);
		if (TB_KineticShield)
		{
			const bool bShowKineticText = bShowKinetic && bShowStatTexts;
			TB_KineticShield->SetVisibility(bShowKineticText ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (bShowKinetic && PB_KineticShield)
		{
			PB_KineticShield->SetPercent(ViewModel.CurrentKineticShields / ViewModel.MaxKineticShields);
			FProgressBarStyle KineticStyle = PB_KineticShield->GetWidgetStyle();
			KineticStyle.FillImage.TintColor = FSlateColor(KineticShieldColor);
			PB_KineticShield->SetWidgetStyle(KineticStyle);
			if (TB_KineticShield)
			{
				TB_KineticShield->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"),
					FMath::RoundToInt32(ViewModel.CurrentKineticShields),
					FMath::RoundToInt32(ViewModel.MaxKineticShields))));
			}
		}
	}

	// Energy shield
	{
		const bool bShowEnergy = ViewModel.MaxEnergyShields > 0.f;
		const ESlateVisibility EnergyVis = bShowEnergy ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		if (SB_EnergyShieldBar)
			SB_EnergyShieldBar->SetVisibility(EnergyVis);
		if (PB_EnergyShield)
			PB_EnergyShield->SetVisibility(EnergyVis);
		if (TB_EnergyShield)
		{
			const bool bShowEnergyText = bShowEnergy && bShowStatTexts;
			TB_EnergyShield->SetVisibility(bShowEnergyText ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (bShowEnergy && PB_EnergyShield)
		{
			PB_EnergyShield->SetPercent(ViewModel.CurrentEnergyShields / ViewModel.MaxEnergyShields);
			FProgressBarStyle EnergyStyle = PB_EnergyShield->GetWidgetStyle();
			EnergyStyle.FillImage.TintColor = FSlateColor(EnergyShieldColor);
			PB_EnergyShield->SetWidgetStyle(EnergyStyle);
			if (TB_EnergyShield)
			{
				TB_EnergyShield->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"),
					FMath::RoundToInt32(ViewModel.CurrentEnergyShields),
					FMath::RoundToInt32(ViewModel.MaxEnergyShields))));
			}
		}
	}

	if (IMG_Border)
	{
		FLinearColor BorderColor;
		switch (ViewModel.Team)
		{
		case ERCombatTeam::Player:  BorderColor = PlayerColor;  break;
		case ERCombatTeam::Enemy:   BorderColor = EnemyColor;   break;
		case ERCombatTeam::Neutral: BorderColor = NeutralColor; break;
		}
		IMG_Border->SetBrushTintColor(FSlateColor(BorderColor));
	}

	WB_StatusEffects->ClearChildren();

	if (!bShowStatusEffects)
	{
		EffectsOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		EffectsOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		// Sort effects: infinite first, then descending by TurnsLeft
		ViewModel.StatusEffects.Sort([](const FEffectIconViewModel& A, const FEffectIconViewModel& B)
		{
			if (A.bIsInfinite != B.bIsInfinite)
			{
				return A.bIsInfinite;
			}
			return A.TurnsLeft > B.TurnsLeft;
		});

		const int32 NumStatusEffectsToShow = FMath::Min(ViewModel.StatusEffects.Num(), MaxVisibleStatusEffects);

		for (int32 i = 0; i < NumStatusEffectsToShow; i++)
		{
			if (EffectIconWidgetClass)
			{
				UREffectIconWidget* NewEffectIcon = CreateWidget<UREffectIconWidget>(this, EffectIconWidgetClass);
				NewEffectIcon->Init(ViewModel.StatusEffects[i]);
				NewEffectIcon->SetVisibility(ESlateVisibility::Visible);
				if (NewEffectIcon && WB_StatusEffects)
				{
					WB_StatusEffects->AddChild(NewEffectIcon);
				}
			}
		}
	}
}
