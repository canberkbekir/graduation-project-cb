// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "REffectIconWidget.generated.h"

/**
 * 
 */
UCLASS()
class RIZZGAME_API UREffectIconWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TB_StackCount;

	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Icon;

public:
	void Init(const struct FEffectIconViewModel& ViewModel) const;
};
