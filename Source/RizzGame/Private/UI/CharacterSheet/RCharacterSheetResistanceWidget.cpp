// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CharacterSheet/RCharacterSheetResistanceWidget.h"

#include "Components/TextBlock.h"

void URCharacterSheetResistanceWidget::SetNetworkResistanceValue(float Value)     { SetValueText(Txt_NetworkResistanceValue, Value); }
void URCharacterSheetResistanceWidget::SetPhysicalResistanceValue(float Value) { SetValueText(Txt_PhysicalResistanceValue, Value); }
void URCharacterSheetResistanceWidget::SetMentalResistanceValue(float Value)  { SetValueText(Txt_MentalResistanceValue, Value); }
void URCharacterSheetResistanceWidget::SetRadiationResistanceValue(float Value)     { SetValueText(Txt_RadiationResistanceValue, Value); }
void URCharacterSheetResistanceWidget::SetElectroMagneticResistanceValue(float Value)  { SetValueText(Txt_ElectroMagneticResistanceValue, Value); }

void URCharacterSheetResistanceWidget::ClearAll()
{
	SetNetworkResistanceValue(0.f);
	SetPhysicalResistanceValue(0.f);
	SetMentalResistanceValue(0.f);
	SetRadiationResistanceValue(0.f);
	SetElectroMagneticResistanceValue(0.f);
}

void URCharacterSheetResistanceWidget::SetValueText(UTextBlock* Text, float Value) const
{
	if (!Text) return;

	const int32 AsInt = FMath::RoundToInt(Value);

	if (bTwoDigits)
	{
		Text->SetText(FText::FromString(FString::Printf(TEXT("%02d"), AsInt)));
	}
	else
	{
		Text->SetText(FText::AsNumber(AsInt));
	}
}