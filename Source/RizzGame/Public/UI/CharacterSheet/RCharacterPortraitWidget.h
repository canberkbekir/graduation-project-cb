#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RCharacterPortraitWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
class UBorder;
class UTexture2D;
struct FStreamableHandle;
struct FCharacterDefinitionRow;

UCLASS()
class RIZZGAME_API URCharacterPortraitWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Sets visuals from a definition row (async-loads soft portrait if needed). */
	UFUNCTION(BlueprintCallable, Category="Rizz|Portrait")
	void InitFromDefinition(const FCharacterDefinitionRow& InDef);

	UFUNCTION(BlueprintCallable, Category="Rizz|Portrait")
	void SetLevel(int32 InLevel);

	UFUNCTION(BlueprintCallable, Category="Rizz|Portrait")
	void SetHealth(float InCurrent, float InMax);

	UFUNCTION(BlueprintCallable, Category="Rizz|Portrait")
	void SetLevelProgress(float NewCurrentExperience, float MaxExperience);

	UFUNCTION(BlueprintCallable, Category="Rizz|Portrait")
	void SetEnergyShield(float CurrentShield, float MaxShield);

	UFUNCTION(BlueprintCallable, Category="Rizz|Portrait")
	void SetPhysicalShield(float CurrentShield, float MaxShield);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	void ApplyFallback();
	void HandlePortraitLoaded();

protected:
	// --- Bind these names in your Widget Blueprint (BindWidgetOptional lets you omit some) ---

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget), Category="Rizz|Portrait")
	TObjectPtr<UImage> Img_Portrait;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Rizz|Portrait")
	TObjectPtr<UBorder> Border_Frame;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Rizz|Portrait")
	TObjectPtr<UTextBlock> Txt_Level;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Rizz|Portrait")
	TObjectPtr<UProgressBar> PB_Health;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Rizz|Portrait")
	TObjectPtr<UProgressBar> PB_PhysicalShield;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Rizz|Portrait")
	TObjectPtr<UProgressBar> PB_EnergyShield;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Rizz|Portrait")
	TObjectPtr<UProgressBar> PB_Experience;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Rizz|Portrait")
	TObjectPtr<UTextBlock> Txt_Health;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Rizz|Portrait")
	TObjectPtr<UTextBlock> Txt_EnergyShield;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Rizz|Portrait")
	TObjectPtr<UTextBlock> Txt_PhysicalShield;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Rizz|Portrait")
	TObjectPtr<UTextBlock> Txt_Name;

private:
	/** Cached portrait soft pointer for async loading */
	TSoftObjectPtr<UTexture2D> CachedPortrait;

	float CurrentHP = 0.f;
	float MaxHP = 0.f;

	float CurrentPhysicalShield = 0.f;
	float MaxPhysicalShield = 0.f;

	float CurrentEnergyShield = 0.f;
	float MaxEnergyShield = 0.f;

	float CurrentExperience = 0.f;
	float ExperienceToLevelUp = 0.f;

	TSharedPtr<FStreamableHandle> PortraitHandle;
};
