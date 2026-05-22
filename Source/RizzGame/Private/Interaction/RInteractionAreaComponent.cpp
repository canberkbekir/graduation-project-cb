// Fill out your copyright notice in the Description page of Project Settings.

#include "Interaction/RInteractionAreaComponent.h"

#include "Interaction/RInteractable.h"
#include "GameFramework/Actor.h"

#if WITH_EDITOR
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#endif

URInteractionAreaComponent::URInteractionAreaComponent()
{
	InitSphereRadius(200.f);
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetGenerateOverlapEvents(false);
	bHiddenInGame = true;
}

float URInteractionAreaComponent::GetInteractionRadius() const
{
	return GetScaledSphereRadius();
}

FVector URInteractionAreaComponent::GetInteractionLocation() const
{
	return GetComponentLocation();
}

bool URInteractionAreaComponent::ValidateInteractableActor(const AActor* Actor, TArray<FText>& OutErrors)
{
	if (!Actor) return false;

	bool bValid = true;

	const bool bImplementsInterface = Actor->GetClass()->ImplementsInterface(URInteractable::StaticClass());

	TArray<URInteractionAreaComponent*> Areas;
	Actor->GetComponents<URInteractionAreaComponent>(Areas);

	if (Areas.Num() > 0 && !bImplementsInterface)
	{
		OutErrors.Add(FText::FromString(FString::Printf(
			TEXT("%s has URInteractionAreaComponent(s) but does not implement IRInteractable."),
			*Actor->GetName())));
		bValid = false;
	}

	if (bImplementsInterface && Areas.Num() == 0)
	{
		OutErrors.Add(FText::FromString(FString::Printf(
			TEXT("%s implements IRInteractable but has no URInteractionAreaComponent. Add at least one."),
			*Actor->GetName())));
		bValid = false;
	}

	return bValid;
}

#if WITH_EDITOR
void URInteractionAreaComponent::CheckForErrors()
{
	Super::CheckForErrors();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	TArray<FText> Errors;
	if (!ValidateInteractableActor(Owner, Errors))
	{
		FMessageLog MapCheck("MapCheck");
		for (const FText& Error : Errors)
		{
			MapCheck.Warning()
				->AddToken(FUObjectToken::Create(Owner))
				->AddToken(FTextToken::Create(Error));
		}
	}
}
#endif
