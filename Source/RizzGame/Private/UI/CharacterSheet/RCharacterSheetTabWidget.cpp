#include "UI/CharacterSheet/RCharacterSheetTabWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

void URCharacterSheetTabWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_SkillsTab)
	{
		Btn_SkillsTab->OnClicked.AddDynamic(this, &URCharacterSheetTabWidget::OnSkillsTabClicked);
	}

	if (Btn_WeaponsTab)
	{
		Btn_WeaponsTab->OnClicked.AddDynamic(this, &URCharacterSheetTabWidget::OnWeaponsTabClicked);
	}
 
	SelectTab(0);
}

void URCharacterSheetTabWidget::SelectTab(int32 Index)
{
	if (SheetSwitcher)
	{
		SheetSwitcher->SetActiveWidgetIndex(Index);
	}

	UpdateTabButtonStyles(Index);
}

void URCharacterSheetTabWidget::UpdateTabButtonStyles(int32 ActiveIndex)
{ 
	if (Btn_SkillsTab)
	{
		Btn_SkillsTab->SetStyle(ActiveIndex == 0 ? TabStyleSelected : TabStyleNormal);
	}

	// Weapons
	if (Btn_WeaponsTab)
	{
		Btn_WeaponsTab->SetStyle(ActiveIndex == 1 ? TabStyleSelected : TabStyleNormal);
	}
}

void URCharacterSheetTabWidget::OnSkillsTabClicked()
{
	SelectTab(0);
}

void URCharacterSheetTabWidget::OnWeaponsTabClicked()
{
	SelectTab(1);
}
