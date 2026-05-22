


#include "CoverPoint.h"
#include "Components/StaticMeshComponent.h"


ACoverPoint::ACoverPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	
	CoverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoverMesh"));
	CoverMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = CoverMesh;
	
	Tags.Add(FName("CoverPoint"));
}

void ACoverPoint::OccupyCover(AActor* NewOccupant)
{
	CoverState = ECoverState::Occupied;
	CoverActor = NewOccupant;
}

void ACoverPoint::ReserveCover(AActor* ReservingActor)
{
	CoverState = ECoverState::Reserved;
	CoverActor = ReservingActor;
}

void ACoverPoint::ReleaseCover()
{
	CoverState = ECoverState::Available;
	CoverActor = nullptr;
}


