// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "RInteractionAreaComponent.generated.h"

/**
 * Reusable interaction area component.
 * Add one or more of these to any actor that implements IRInteractable.
 * All areas on the same actor share the actor's interaction properties.
 * RGA_Interact will pick the closest reachable area automatically.
 */
UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class RIZZGAME_API URInteractionAreaComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	URInteractionAreaComponent();

	/** Returns the interaction radius (scaled sphere radius). */
	UFUNCTION(BlueprintCallable, Category = "Rizz|Interaction")
	float GetInteractionRadius() const;

	/** Returns the world-space interaction location (component location). */
	UFUNCTION(BlueprintCallable, Category = "Rizz|Interaction")
	FVector GetInteractionLocation() const;

	/**
	 * Validates that the owning actor implements IRInteractable.
	 * Also checks placed actors for missing interaction area components.
	 */
	static bool ValidateInteractableActor(const AActor* Actor, TArray<FText>& OutErrors);

#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif
};
