// RCharacterSheetSkillTabWidget.cpp

#include "UI/CharacterSheet/RCharacterSheetSkillTabWidget.h"
#include "Components/TextBlock.h"
#include "Math/UnrealMathUtility.h"

void URCharacterSheetSkillTabWidget::SetValueText(UTextBlock* Text, float Value) const
{
	if (!Text) return;

	const int32 IntValue = FMath::RoundToInt(Value);

	if (bTwoDigits)
	{
		Text->SetText(FText::FromString(FString::Printf(TEXT("%02d"), IntValue)));
	}
	else
	{
		Text->SetText(FText::AsNumber(IntValue));
	}
}

void URCharacterSheetSkillTabWidget::SetNetworkAbilities(float Value) { SetValueText(Txt_NetworkAbilitiesValue, Value); }
void URCharacterSheetSkillTabWidget::SetHandleTech(float Value)       { SetValueText(Txt_HandleTechValue, Value); }
void URCharacterSheetSkillTabWidget::SetHealthBonus(float Value)      { SetValueText(Txt_HealthBonusValue, Value); }
void URCharacterSheetSkillTabWidget::SetEvasion(float Value)          { SetValueText(Txt_EvasionValue, Value); }
void URCharacterSheetSkillTabWidget::SetMovement(float Value)         { SetValueText(Txt_MovementValue, Value); }

void URCharacterSheetSkillTabWidget::SetInvestigation(float Value)    { SetValueText(Txt_InvestigationValue, Value); }
void URCharacterSheetSkillTabWidget::SetIntelligence(float Value)     { SetValueText(Txt_IntelligenceValue, Value); }
void URCharacterSheetSkillTabWidget::SetInitiative(float Value)       { SetValueText(Txt_InitiativeValue, Value); }
void URCharacterSheetSkillTabWidget::SetPersuasion(float Value)       { SetValueText(Txt_PersuasionValue, Value); }
void URCharacterSheetSkillTabWidget::SetPerception(float Value)       { SetValueText(Txt_PerceptionValue, Value); }

void URCharacterSheetSkillTabWidget::ClearAll()
{
	SetNetworkAbilities(0.f);
	SetHandleTech(0.f);
	SetHealthBonus(0.f);
	SetEvasion(0.f);
	SetMovement(0.f);

	SetInvestigation(0.f);
	SetIntelligence(0.f);
	SetInitiative(0.f);
	SetPersuasion(0.f);
	SetPerception(0.f);
}
