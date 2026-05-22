// RGameHUD.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Overlay.h"
#include "Components/PanelWidget.h"
#include "RGameHUD.generated.h"

UENUM(BlueprintType)
enum class ERUILayer : uint8
{
	Exploration,
	Combat,
	Overlay,
	Popup
};

UCLASS()
class RIZZGAME_API URGameHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "R|UI")
	void SetLayerVisible(ERUILayer Layer, bool bVisible,
		ESlateVisibility VisType = ESlateVisibility::SelfHitTestInvisible);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "R|UI")
	bool IsLayerVisible(ERUILayer Layer) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "R|UI")
	UPanelWidget* GetLayer(ERUILayer Layer) const;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "R|UI")
	TObjectPtr<UOverlay> L_Exploration = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "R|UI")
	TObjectPtr<UOverlay> L_Combat = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "R|UI")
	TObjectPtr<UOverlay> L_Overlay = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "R|UI")
	TObjectPtr<UOverlay> L_Popup = nullptr;
};
