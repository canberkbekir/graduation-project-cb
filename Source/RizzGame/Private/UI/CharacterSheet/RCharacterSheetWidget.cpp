// RCharacterSheetWidget.cpp

#include "UI/CharacterSheet/RCharacterSheetWidget.h"
#include "UI/CharacterSheet/RCharacterPortraitWidget.h"
#include "UI/CharacterSheet/RCharacterSheetAbilitiesWidget.h"
#include "UI/CharacterSheet/RCharacterSheetResistanceWidget.h"
#include "UI/CharacterSheet/RCharacterSheetTabWidget.h"
#include "Core/RCharacterBase.h"
#include "Character/RCharacterSpec.h"
#include "GAS/Attributes/RCharacterCoreAttributeSet.h"
#include "GAS/Attributes/RCharacterCombatAttributeSet.h"

void URCharacterSheetWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void URCharacterSheetWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void URCharacterSheetWidget::InitFromCharacter(ARCharacterBase* InCharacter)
{
    Character = InCharacter;
    Refresh();
}

void URCharacterSheetWidget::InitFromDefinition(const FCharacterDefinitionRow& InDef)
{
    Character = nullptr;

    if (PortraitWidget)
    {
        PortraitWidget->InitFromDefinition(InDef);
    }
}

void URCharacterSheetWidget::Refresh()
{
    UpdatePortrait();
    UpdateAbilities();
    UpdateResistances();
}

void URCharacterSheetWidget::Clear()
{
    Character = nullptr;

    if (AbilitiesWidget)
    {
        AbilitiesWidget->ClearAll();
    }

    if (ResistanceWidget)
    {
        ResistanceWidget->ClearAll();
    }
}

void URCharacterSheetWidget::PlaySlideInAnimation()
{
    BP_PlaySlideInAnimation();
}

void URCharacterSheetWidget::PlaySlideOutAnimation()
{
    BP_PlaySlideOutAnimation();
}

void URCharacterSheetWidget::UpdatePortrait()
{
    if (!PortraitWidget || !Character)
    {
        return;
    }

    // Init portrait from character's DataTable row
    const FDataTableRowHandle& Row = Character->GetCharacterRow();
    if (Row.DataTable && !Row.RowName.IsNone())
    {
        if (const FCharacterDefinitionRow* Def = Row.DataTable->FindRow<FCharacterDefinitionRow>(
            Row.RowName, TEXT("URCharacterSheetWidget::UpdatePortrait")))
        {
            PortraitWidget->InitFromDefinition(*Def);
        }
    }

    // Get combat attributes for health/shields
    if (URCharacterCombatAttributeSet* CombatAttribs = Character->GetCombatAttributes())
    {
        float CurrentHP = CombatAttribs->GetHealth();
        float MaxHP = CombatAttribs->GetMaxHealth();
        PortraitWidget->SetHealth(CurrentHP, MaxHP);

        float KineticShield = CombatAttribs->GetKineticShields();
        float MaxKineticShield = CombatAttribs->GetMaxKineticShields();
        PortraitWidget->SetPhysicalShield(KineticShield, MaxKineticShield);

        float EnergyShield = CombatAttribs->GetEnergyShields();
        float MaxEnergyShield = CombatAttribs->GetMaxEnergyShields();
        PortraitWidget->SetEnergyShield(EnergyShield, MaxEnergyShield);
    }

    // TODO: Get level from character
    PortraitWidget->SetLevel(1);
}

void URCharacterSheetWidget::UpdateAbilities()
{
    if (!AbilitiesWidget || !Character)
    {
        return;
    }

    if (URCharacterCoreAttributeSet* CoreAttribs = Character->GetCoreAttributes())
    {
        AbilitiesWidget->SetTechValue(CoreAttribs->GetTech());
        AbilitiesWidget->SetPhysiqueValue(CoreAttribs->GetPhysique());
        AbilitiesWidget->SetFinesseValue(CoreAttribs->GetFinesse());
        AbilitiesWidget->SetMindValue(CoreAttribs->GetMind());
        AbilitiesWidget->SetInsightValue(CoreAttribs->GetInsight());
    }
}

void URCharacterSheetWidget::UpdateResistances()
{
    if (!ResistanceWidget || !Character)
    {
        return;
    }

    // TODO: Get resistance values from character attributes when available
    ResistanceWidget->ClearAll();
}
