// Fill out your copyright notice in the Description page of Project Settings.

 
#include "UI/Common/REffectIconWidget.h" 
#include "UI/Models/RCombatUIModels.h"

void UREffectIconWidget::Init(const struct FEffectIconViewModel& ViewModel) const
{
	IMG_Icon->SetBrushFromTexture(ViewModel.Icon);
	IMG_Icon->SetColorAndOpacity(ViewModel.TintColor);

	if (ViewModel.bIsInfinite)
	{
		TB_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
	else if (ViewModel.TurnsLeft > 0)
	{
		TB_StackCount->SetText(FText::AsNumber(ViewModel.TurnsLeft));
		TB_StackCount->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		TB_StackCount->SetText(FText::AsNumber(ViewModel.Stacks));
		TB_StackCount->SetVisibility(ViewModel.Stacks > 1 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}
