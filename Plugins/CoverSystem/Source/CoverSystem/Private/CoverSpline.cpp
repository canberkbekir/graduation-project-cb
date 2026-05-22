


#include "CoverSpline.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

ACoverSpline::ACoverSpline()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	RootComponent = SplineComponent;
}

void ACoverSpline::GenerateCoverPoints()
{
	ClearCoverPoints();

	if (!SplineComponent || !CoverPointClass)
		return;

	const auto numPoints = SplineComponent->GetNumberOfSplinePoints();

	for (auto i = 0; i < numPoints; i++)
	{
		FVector pointLocation = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ACoverPoint* newCoverPoint = GetWorld()->SpawnActor<ACoverPoint>(CoverPointClass, pointLocation, FRotator::ZeroRotator, spawnParams);

		if (newCoverPoint)
		{
			if (CoverPointMesh && newCoverPoint->CoverMesh)
			{
				newCoverPoint->CoverMesh->SetStaticMesh(CoverPointMesh);

				float Scale = SphereRadius / 50.0f;
				newCoverPoint->SetActorScale3D(FVector(Scale));
			}
			newCoverPoint->CoverType = CoverType;
			newCoverPoint->CrouchPosition = CrouchPosition;
			newCoverPoint->StandPosition = StandPosition;
			newCoverPoint->bFindOutPositionDynamically = bFindOutPositionDynamically;
			
			newCoverPoint->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
#if WITH_EDITOR
			newCoverPoint->SetActorLabel(FString::Printf(TEXT("CoverPoint_%d"), i));
#endif
			SpawnedCoverPoints.Add(newCoverPoint);
		}
	}
}

void ACoverSpline::ClearCoverPoints()
{
	for (auto coverPoint : SpawnedCoverPoints)
		if (coverPoint)
			coverPoint->Destroy();
	
	SpawnedCoverPoints.Empty();
}
