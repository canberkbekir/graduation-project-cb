#include "UI/CharacterSheet/RCharacterSheetAbilitiesWidget.h"

#include "Components/TextBlock.h"
#include "Math/UnrealMathUtility.h"

void URCharacterSheetAbilitiesWidget::SetTechValue(float Value)     { SetValueText(Txt_TechValue, Value); }
void URCharacterSheetAbilitiesWidget::SetPhysiqueValue(float Value) { SetValueText(Txt_PhysiqueValue, Value); }
void URCharacterSheetAbilitiesWidget::SetFinesseValue(float Value)  { SetValueText(Txt_FinesseValue, Value); }
void URCharacterSheetAbilitiesWidget::SetMindValue(float Value)     { SetValueText(Txt_MindValue, Value); }
void URCharacterSheetAbilitiesWidget::SetInsightValue(float Value)  { SetValueText(Txt_InsightValue, Value); }

void URCharacterSheetAbilitiesWidget::ClearAll()
{
	SetTechValue(0.f);
	SetPhysiqueValue(0.f);
	SetFinesseValue(0.f);
	SetMindValue(0.f);
	SetInsightValue(0.f);
}

void URCharacterSheetAbilitiesWidget::SetValueText(UTextBlock* Text, float Value) const
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
