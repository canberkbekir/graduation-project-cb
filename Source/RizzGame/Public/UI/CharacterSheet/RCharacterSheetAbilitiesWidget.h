#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RCharacterSheetAbilitiesWidget.generated.h"

class UTextBlock;

UCLASS()
class RIZZGAME_API URCharacterSheetAbilitiesWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Manual setters (no GAS yet)
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetTechValue(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetPhysiqueValue(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetFinesseValue(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetMindValue(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetInsightValue(float Value);

	// Optional convenience
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void ClearAll();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|UI|Abilities")
	bool bTwoDigits = true;

protected:
	// BindWidget texts
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_TechValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_PhysiqueValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_FinesseValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_MindValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_InsightValue;

private:
	void SetValueText(UTextBlock* Text, float Value) const;
};
