// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ISelectable.generated.h"

UENUM(BlueprintType)
enum class ESelectableType : uint8
{
	PartyMember,
	Enemy,
	Interactable
};

UINTERFACE(Blueprintable, MinimalAPI)
class URSelectable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implemented by any actor that can be hovered or selected by the player controller.
 * Works alongside URSelectionComponent (visuals) — this interface provides callbacks.
 */
class RIZZGAME_API IRSelectable
{
	GENERATED_BODY()

public:
	/** What kind of selectable this actor is — drives click behavior in RPlayerController. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Selection")
	ESelectableType GetSelectableType() const;

	/** Called when the cursor moves onto this actor. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Selection")
	void OnHoverBegin();

	/** Called when the cursor moves off this actor. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Selection")
	void OnHoverEnd();

	/** Called when this actor becomes the active selection. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Selection")
	void OnSelected();

	/** Called when this actor is deselected. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Selection")
	void OnDeselected();
};
