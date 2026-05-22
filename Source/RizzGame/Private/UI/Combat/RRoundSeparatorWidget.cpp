// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Combat/RRoundSeparatorWidget.h"

void URRoundSeparatorWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (SB_Root && SeparatorWidth > 0.f)
	{
		SB_Root->SetWidthOverride(SeparatorWidth);
	}
}

void URRoundSeparatorWidget::SetRound(const int32 Round)
{
	 
}
