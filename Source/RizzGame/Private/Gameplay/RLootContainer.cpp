#include "Gameplay/RLootContainer.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Interaction/RInteractionAreaComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/RInventoryComponent.h"
#include "Items/RItemDefinition.h"

ARLootContainer::ARLootContainer()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	PrimaryActorTick.bCanEverTick = true;
#else
	PrimaryActorTick.bCanEverTick = false;
#endif

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ContainerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContainerMesh"));
	ContainerMesh->SetupAttachment(Root);

	InteractionArea = CreateDefaultSubobject<URInteractionAreaComponent>(TEXT("InteractionArea"));
	InteractionArea->SetupAttachment(Root);

	bHasBeenOpened = false;
}

void ARLootContainer::BeginPlay()
{
	Super::BeginPlay();
	bHasBeenOpened = bStartsOpened;
}

void ARLootContainer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (bShowInteractionArea && InteractionArea && GetWorld())
	{
		const FVector Center = InteractionArea->GetInteractionLocation();
		const float Radius   = InteractionArea->GetInteractionRadius();

		DrawDebugSphere(
			GetWorld(),
			Center,
			Radius,
			32,
			FColor::Cyan,
			false,
			0.f,
			0,
			1.f
		);
	}
#endif
}

/* ---------- Loot / state API ---------- */

void ARLootContainer::GetLootEntries(TArray<FLootEntry>& OutEntries) const
{
	OutEntries = Loot.Entries;
}

bool ARLootContainer::IsEmpty() const
{
	return Loot.Entries.Num() == 0;
}

void ARLootContainer::ClearLoot(AActor* InteractingActor)
{
	if (IsEmpty())
	{
		return;
	}

	Loot.Entries.Reset();
	BP_OnLootEmptied(InteractingActor);
}

void ARLootContainer::SetLocked(bool bNewLocked)
{
	bIsLocked = bNewLocked;
}

bool ARLootContainer::TryOpen(AActor* InteractingActor)
{
	if (!InteractingActor)
	{
		return false;
	}

	if (bIsLocked)
	{
		bool bUnlocked = RequiredKeyItem
			? BP_TryUseRequiredKey(InteractingActor)
			: BP_TryAlternativeUnlock(InteractingActor);

		if (bUnlocked)
		{
			bIsLocked = false;
		}
		else
		{
			BP_OnOpenFailedLocked(InteractingActor);
			return false;
		}
	}

	const bool bWasOpenedBefore = bHasBeenOpened;
	bHasBeenOpened = true;

	if (!bWasOpenedBefore)
	{
		BP_OnOpenedFirstTime(InteractingActor);
	}

	BP_OnOpened(InteractingActor);

	if (bConsumeLootOnFirstOpen && !bWasOpenedBefore && !IsEmpty())
	{
		TakeAllLoot(InteractingActor);
	}

	return true;
}

bool ARLootContainer::TakeAllLoot(AActor* InteractingActor)
{
	if (!InteractingActor || IsEmpty())
	{
		return false;
	}

	URInventoryComponent* Inventory = InteractingActor->FindComponentByClass<URInventoryComponent>();
	if (!Inventory)
	{
		return false;
	}

	bool bAnyTaken = false;

	for (int32 i = Loot.Entries.Num() - 1; i >= 0; --i)
	{
		const FLootEntry& Entry = Loot.Entries[i];
		if (!Entry.Item || Entry.Quantity <= 0)
		{
			Loot.Entries.RemoveAt(i);
			continue;
		}

		if (Inventory->TryAddItem(Entry.Item, Entry.Quantity))
		{
			Loot.Entries.RemoveAt(i);
			bAnyTaken = true;
		}
	}

	if (bAnyTaken && IsEmpty())
	{
		BP_OnLootEmptied(InteractingActor);
	}

	return bAnyTaken;
}

/* ---------- IRInteractable ---------- */

bool ARLootContainer::CanInteract_Implementation(APawn* InteractingPawn)
{
	if (!InteractingPawn)
	{
		return false;
	}

	const FVector PawnLocation = InteractingPawn->GetActorLocation();
	const FVector Center       = IRInteractable::Execute_GetInteractionLocation(this);
	const float   Radius       = IRInteractable::Execute_GetInteractionRadius(this);

	const float DistSq = FVector::DistSquared(PawnLocation, Center);
	return DistSq <= FMath::Square(Radius);
}

void ARLootContainer::Interact_Implementation(APawn* InteractingPawn)
{
	if (!InteractingPawn)
	{
		return;
	}

	TryOpen(InteractingPawn);
}

#define LOCTEXT_NAMESPACE "RLootContainer"

FText ARLootContainer::GetInteractionName_Implementation()
{
	if (IsLocked())
	{
		return LOCTEXT("NameLocked", "Locked Container");
	}

	if (HasBeenOpened() && IsEmpty())
	{
		return LOCTEXT("NameEmpty", "Empty Container");
	}

	return LOCTEXT("NameDefault", "Container");
}

FText ARLootContainer::GetInteractionAction_Implementation()
{
	if (IsLocked())
	{
		if (RequiredKeyItem)
		{
			return FText::Format(LOCTEXT("ActionUseKeyNamed", "Use {0}"), RequiredKeyItem->Visual.ItemName);
		}
		return LOCTEXT("ActionLocked", "Locked");
	}

	if (!HasBeenOpened())
	{
		return LOCTEXT("ActionOpen", "Open");
	}

	if (!IsEmpty())
	{
		return LOCTEXT("ActionLoot", "Loot");
	}

	return LOCTEXT("ActionInspect", "Inspect");
}

#undef LOCTEXT_NAMESPACE

float ARLootContainer::GetInteractionRadius_Implementation()
{
	if (InteractionArea)
	{
		return InteractionArea->GetInteractionRadius();
	}

	return 200.f;
}

FVector ARLootContainer::GetInteractionLocation_Implementation()
{
	return InteractionArea ? InteractionArea->GetInteractionLocation() : GetActorLocation();
}

bool ARLootContainer::BP_TryUseRequiredKey_Implementation(AActor* InteractingActor)
{
	if (!InteractingActor || !RequiredKeyItem)
	{
		return false;
	}

	URInventoryComponent* Inventory = InteractingActor->FindComponentByClass<URInventoryComponent>();
	if (!Inventory || !Inventory->HasItem(RequiredKeyItem, 1))
	{
		return false;
	}

	if (bConsumeKeyOnUse)
	{
		Inventory->RemoveQuantityOfDefinition(RequiredKeyItem, 1);
	}

	return true;
}

bool ARLootContainer::BP_TryAlternativeUnlock_Implementation(AActor* InteractingActor)
{
	return false;
}
