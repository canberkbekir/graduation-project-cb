// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Events/CombatEvents.h"
#include "UI/Common/RPortraitWidget.h"
#include "UI/Combat/RRoundSeparatorWidget.h"
#include "RCombatInitiativeQueueWidget.generated.h"

UCLASS()
class RIZZGAME_API URCombatInitiativeQueueWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Initiative")
	int32 MaxVisibleEntries = 5;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* HB_Root;

	UPROPERTY(EditAnywhere, Category="Initiative", meta=(AllowPrivateAccess="true"))
	TSubclassOf<URPortraitWidget> PortraitWidgetClass;

	UPROPERTY(EditAnywhere, Category="Initiative", meta=(AllowPrivateAccess="true"))
	TSubclassOf<URRoundSeparatorWidget> RoundSeparatorClass;

	/** Whether to show round separator text between rounds. */
	UPROPERTY(EditAnywhere, Category="Rizz")
	bool bShowRoundText = true;

public:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

private:
	void OnInitiativeChanged(const FCombatInitiativeChanged& Event);
};
