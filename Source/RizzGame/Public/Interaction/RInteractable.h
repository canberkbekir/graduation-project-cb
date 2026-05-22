// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RInteractable.generated.h"

class APawn;
 
UINTERFACE(Blueprintable)
class URInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RIZZGAME_API IRInteractable
{
	GENERATED_BODY() 
public:
	/** Can this pawn interact with us right now? (distance, state, etc.) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(APawn* InteractingPawn);

	/** Called when interaction actually happens (open chest, talk, etc.) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(APawn* InteractingPawn);

	/** For UI: "Chest", "Door", "Merchant", etc. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionName();

	/** For UI: "Open", "Talk", "Loot", etc. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionAction();

	/** Interaction radius used for click-to-move logic. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	float GetInteractionRadius();

	/** Where the pawn should stand / move to for interaction. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FVector GetInteractionLocation();

};