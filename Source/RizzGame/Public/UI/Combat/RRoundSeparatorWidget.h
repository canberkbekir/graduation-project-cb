// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "RRoundSeparatorWidget.generated.h"

UCLASS()
class RIZZGAME_API URRoundSeparatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetRound(int32 Round);

protected:
	virtual void NativePreConstruct() override;

	UPROPERTY(meta = (BindWidget))
	USizeBox* SB_Root;

	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Separator; 
	/** Width of the separator in pixels. */
	UPROPERTY(EditAnywhere, Category="Rizz")
	float SeparatorWidth = 24.f;
};
