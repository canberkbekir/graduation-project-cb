// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RSelectionComponent.generated.h"

/**
 * Visual states driven by the selection system.
 * Each state maps to a Custom Depth stencil value for post-process outline rendering.
 *
 * To see outlines in-game, add a Post Process Volume to your level and configure a
 * custom post-process material that reads CustomDepth stencil values 1/2/3.
 */
UENUM(BlueprintType)
enum class ESelectionVisualState : uint8
{
	None        UMETA(DisplayName="None"),
	Hovered     UMETA(DisplayName="Hovered"),
	Selected    UMETA(DisplayName="Selected"),
	ActiveTurn  UMETA(DisplayName="Active Turn")
};

/**
 * Add this component to any actor that should display a hover/selection outline.
 * The component iterates all UPrimitiveComponents on the owner and sets Custom Depth
 * stencil values to drive post-process outline effects.
 *
 * Stencil values (configurable per component instance):
 *   Hovered    = 1  (e.g. white outline)
 *   Selected   = 2  (e.g. green/yellow outline)
 *   ActiveTurn = 3  (e.g. gold outline — whose turn it is in combat)
 */
UCLASS(Blueprintable, ClassGroup=(Rizz), meta=(BlueprintSpawnableComponent))
class RIZZGAME_API URSelectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URSelectionComponent();

	/** Set the current visual state. Applies stencil values to all primitive components on the owner. */
	UFUNCTION(BlueprintCallable, Category="Selection")
	void SetVisualState(ESelectionVisualState NewState);

	UFUNCTION(BlueprintPure, Category="Selection")
	ESelectionVisualState GetVisualState() const { return CurrentState; }

	// --- Configurable stencil values ---

	/** Stencil value written when state is Hovered. Match in your post-process material. */
	UPROPERTY(EditDefaultsOnly, Category="Selection|Stencil")
	int32 HoveredStencilValue = 1;

	/** Stencil value written when state is Selected. */
	UPROPERTY(EditDefaultsOnly, Category="Selection|Stencil")
	int32 SelectedStencilValue = 2;

	/** Stencil value written when state is ActiveTurn (combat only). */
	UPROPERTY(EditDefaultsOnly, Category="Selection|Stencil")
	int32 ActiveTurnStencilValue = 3;

private:
	ESelectionVisualState CurrentState = ESelectionVisualState::None;

	/** Applies or removes Custom Depth rendering + stencil value on all primitives. */
	void ApplyStencil(int32 StencilValue, bool bEnable);
};
