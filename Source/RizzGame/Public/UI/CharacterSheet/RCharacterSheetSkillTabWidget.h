// RCharacterSheetSkillTabWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RCharacterSheetSkillTabWidget.generated.h"

class UTextBlock;

UCLASS()
class RIZZGAME_API URCharacterSheetSkillTabWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetNetworkAbilities(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetHandleTech(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetHealthBonus(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetEvasion(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetMovement(float Value);

	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetInvestigation(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetIntelligence(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetInitiative(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetPersuasion(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetPerception(float Value);

	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void ClearAll();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|UI|Abilities")
	bool bTwoDigits = true;

protected: 
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_NetworkAbilitiesValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_HandleTechValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_HealthBonusValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_EvasionValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_MovementValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_InvestigationValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_IntelligenceValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_InitiativeValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_PersuasionValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_PerceptionValue;

private:
	void SetValueText(UTextBlock* Text, float Value) const;
};
