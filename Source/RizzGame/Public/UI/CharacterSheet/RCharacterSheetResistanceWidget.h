// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RCharacterSheetResistanceWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class RIZZGAME_API URCharacterSheetResistanceWidget : public UUserWidget
{
	GENERATED_BODY()


public:
	// Manual setters (no GAS yet)
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetNetworkResistanceValue(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetPhysicalResistanceValue(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetMentalResistanceValue(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetRadiationResistanceValue(float Value);
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void SetElectroMagneticResistanceValue(float Value);

	// Optional convenience
	UFUNCTION(BlueprintCallable, Category="Rizz|UI|Abilities") void ClearAll();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|UI|Abilities")
	bool bTwoDigits = true;

protected:
	// BindWidget texts
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_NetworkResistanceValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_PhysicalResistanceValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_MentalResistanceValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_RadiationResistanceValue;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Rizz|UI|Abilities")
	TObjectPtr<UTextBlock> Txt_ElectroMagneticResistanceValue;

private:
	void SetValueText(UTextBlock* Text, float Value) const;
};
