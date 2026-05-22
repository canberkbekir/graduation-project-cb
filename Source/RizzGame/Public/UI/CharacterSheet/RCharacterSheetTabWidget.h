// RCharacterSheetTabWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateTypes.h"          // FButtonStyle
#include "RCharacterSheetTabWidget.generated.h"

class UButton;
class UWidgetSwitcher;

UCLASS()
class RIZZGAME_API URCharacterSheetTabWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Btn_SkillsTab;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Btn_WeaponsTab;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UWidgetSwitcher> SheetSwitcher;
 
	UPROPERTY(EditDefaultsOnly, Category="Rizz|Tabs")
	FButtonStyle TabStyleNormal;

	UPROPERTY(EditDefaultsOnly, Category="Rizz|Tabs")
	FButtonStyle TabStyleSelected;

private:
	UFUNCTION() void OnSkillsTabClicked();
	UFUNCTION() void OnWeaponsTabClicked();

	void SelectTab(int32 Index);
	void UpdateTabButtonStyles(int32 ActiveIndex);
};
