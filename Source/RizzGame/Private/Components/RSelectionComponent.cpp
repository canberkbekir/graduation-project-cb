// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/RSelectionComponent.h"
#include "Components/PrimitiveComponent.h"

URSelectionComponent::URSelectionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URSelectionComponent::SetVisualState(ESelectionVisualState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;

	switch (CurrentState)
	{
	case ESelectionVisualState::Hovered:
		ApplyStencil(HoveredStencilValue, true);
		break;
	case ESelectionVisualState::Selected:
		ApplyStencil(SelectedStencilValue, true);
		break;
	case ESelectionVisualState::ActiveTurn:
		ApplyStencil(ActiveTurnStencilValue, true);
		break;
	case ESelectionVisualState::None:
	default:
		ApplyStencil(0, false);
		break;
	}
}

void URSelectionComponent::ApplyStencil(int32 StencilValue, bool bEnable)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	TArray<UPrimitiveComponent*> Primitives;
	Owner->GetComponents<UPrimitiveComponent>(Primitives);

	for (UPrimitiveComponent* Prim : Primitives)
	{
		if (!Prim)
		{
			continue;
		}
		Prim->SetRenderCustomDepth(bEnable);
		if (bEnable)
		{
			Prim->SetCustomDepthStencilValue(StencilValue);
		}
	}
}
