// RDoor.cpp
#include "FogOfWar/RDoor.h"
DEFINE_LOG_CATEGORY_STATIC(LogFogOfWarDoor, Log, All);
#include "FogOfWar/RFogRoom.h"
#include "Subsystems/RFogOfWarSubsystem.h"
#include "Subsystems/REventBusSubsystem.h"
#include "Events/FogOfWarEvents.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

ARDoor::ARDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(Root);

	InteractionArea = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionArea"));
	InteractionArea->SetupAttachment(Root);
	InteractionArea->InitSphereRadius(200.0f);
	InteractionArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionArea->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionArea->SetGenerateOverlapEvents(false);
	InteractionArea->bHiddenInGame = true;
}

void ARDoor::BeginPlay()
{
	Super::BeginPlay();

	if (bStartsOpen)
	{
		TryOpen(nullptr);
	}
	else if (bBlocksLOSWhenClosed)
	{
		SetLOSCollision(true);
	}
}

bool ARDoor::TryOpen(AActor* InteractingActor)
{
	if (bIsOpen)
	{
		return false;
	}

	if (bIsLocked)
	{
		bool bUnlocked = false;

		if (RequiredKeyItem && InteractingActor)
		{
			bUnlocked = BP_TryUseRequiredKey(InteractingActor);
			if (bUnlocked)
			{
				bIsLocked = false;
			}
		}

		if (!bUnlocked)
		{
			UE_LOG(LogFogOfWarDoor, Log, TEXT("Door '%s' open failed — locked (no key)"), *GetName());
			if (InteractingActor)
			{
				BP_OnDoorOpenFailedLocked(InteractingActor);
			}
			return false;
		}
	}

	bIsOpen = true;

	if (bBlocksLOSWhenClosed)
	{
		SetLOSCollision(false);
	}

	UE_LOG(LogFogOfWarDoor, Log, TEXT("Door '%s' [%s] opened by '%s' | revealing %d room(s)"),
		*GetName(), *DoorId.ToString(),
		InteractingActor ? *InteractingActor->GetName() : TEXT("none"),
		ConnectedRooms.Num());

	URFogOfWarSubsystem* FogSubsystem = GetWorld()->GetSubsystem<URFogOfWarSubsystem>();
	if (FogSubsystem)
	{
		for (ARFogRoom* Room : ConnectedRooms)
		{
			FogSubsystem->RevealRoom(Room);
		}
	}
	else
	{
		UE_LOG(LogFogOfWarDoor, Warning, TEXT("Door '%s': URFogOfWarSubsystem not found — rooms not revealed"), *GetName());
	}

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UREventBusSubsystem* EventBus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			FRDoorOpened Event;
			Event.DoorId           = DoorId;
			Event.InteractingActor = InteractingActor;
			EventBus->Publish(Event);
		}
	}

	BP_OnDoorOpened(InteractingActor);
	return true;
}

void ARDoor::Close()
{
	if (!bIsOpen)
	{
		return;
	}

	bIsOpen = false;

	if (bBlocksLOSWhenClosed)
	{
		SetLOSCollision(true);
	}

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UREventBusSubsystem* EventBus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			FRDoorClosed Event;
			Event.DoorId = DoorId;
			EventBus->Publish(Event);
		}
	}

	BP_OnDoorClosed();
}

void ARDoor::SetLOSCollision(bool bBlock)
{
	if (!DoorMesh)
	{
		return;
	}

	DoorMesh->SetCollisionResponseToChannel(ECC_Visibility, bBlock ? ECR_Block : ECR_Ignore);
}

bool ARDoor::CanInteract_Implementation(APawn* InteractingPawn)
{
	return !bIsOpen;
}

void ARDoor::Interact_Implementation(APawn* InteractingPawn)
{
	if (InteractingPawn)
	{
		TryOpen(InteractingPawn);
	}
}

FText ARDoor::GetInteractionName_Implementation()
{
	return bIsLocked ? FText::FromString(TEXT("Locked Door")) : FText::FromString(TEXT("Door"));
}

FText ARDoor::GetInteractionAction_Implementation()
{
	return bIsLocked ? FText::FromString(TEXT("Locked")) : FText::FromString(TEXT("Open"));
}

float ARDoor::GetInteractionRadius_Implementation()
{
	return InteractionArea ? InteractionArea->GetScaledSphereRadius() : 200.0f;
}

FVector ARDoor::GetInteractionLocation_Implementation()
{
	return InteractionArea ? InteractionArea->GetComponentLocation() : GetActorLocation();
}
