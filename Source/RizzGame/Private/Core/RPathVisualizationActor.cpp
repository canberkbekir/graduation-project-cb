#include "Core/RPathVisualizationActor.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Settings/RPathVisualizationConfig.h"

template<typename TComp>
static void ResizePool(TArray<TObjectPtr<TComp>>& Pool, int32 Count, USceneComponent* Parent, AActor* Owner)
{
	while (Pool.Num() < Count)
	{
		TComp* Comp = NewObject<TComp>(Owner);
		Comp->SetupAttachment(Parent);
		Comp->SetMobility(EComponentMobility::Movable);
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Comp->SetVisibility(false);
		Comp->RegisterComponent();
		Pool.Add(Comp);
	}
	for (int32 i = Count; i < Pool.Num(); ++i)
		if (Pool[i]) Pool[i]->SetVisibility(false);
}

APathVisualizationActor::APathVisualizationActor()
{
	PrimaryActorTick.bCanEverTick = false;

	PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PathSpline"));
	SetRootComponent(PathSpline);
}

void APathVisualizationActor::BuildPath(const TArray<FVector>& WorldPoints, URPathVisualizationConfig* Config)
{
	if (!Config || WorldPoints.Num() < 2 || Config->PathSegmentMesh.IsNull())
		return;

	UStaticMesh*        SegMesh = Config->PathSegmentMesh.LoadSynchronous();
	UMaterialInterface* SegMat  = Config->PathMaterial.IsNull()
	                              ? nullptr : Config->PathMaterial.LoadSynchronous();
	const ESplineMeshAxis::Type Axis = static_cast<ESplineMeshAxis::Type>(
		FMath::Clamp(Config->ForwardAxis, 0, 2));

	// Actor is at world origin — world positions == local positions for the spline
	PathSpline->ClearSplinePoints(false);
	for (const FVector& Pt : WorldPoints)
		PathSpline->AddSplineWorldPoint(Pt);
	PathSpline->UpdateSpline();

	const int32 SegCount = WorldPoints.Num() - 1;
	ResizePool(PathSegments, SegCount, PathSpline, this);

	for (int32 i = 0; i < SegCount; ++i)
	{
		USplineMeshComponent* Seg = PathSegments[i];
		FVector StartPos, StartTan, EndPos, EndTan;
		PathSpline->GetLocationAndTangentAtSplinePoint(i,     StartPos, StartTan, ESplineCoordinateSpace::Local);
		PathSpline->GetLocationAndTangentAtSplinePoint(i + 1, EndPos,   EndTan,   ESplineCoordinateSpace::Local);
		Seg->SetStartAndEnd(StartPos, StartTan, EndPos, EndTan, false);
		Seg->SetForwardAxis(Axis, false);
		Seg->SetStartScale(Config->MeshScale, false);
		Seg->SetEndScale(Config->MeshScale, false);
		Seg->SetStartOffset(Config->MeshOffset, false);
		Seg->SetEndOffset(Config->MeshOffset, true);
		Seg->SetStaticMesh(SegMesh);
		if (SegMat) Seg->SetMaterial(0, SegMat);
		Seg->SetVisibility(true);
	}

	UStaticMesh*        WpMesh = (Config->bShowWaypoints && !Config->WaypointMesh.IsNull())
	                             ? Config->WaypointMesh.LoadSynchronous() : nullptr;
	UMaterialInterface* WpMat  = (WpMesh && !Config->WaypointMaterial.IsNull())
	                             ? Config->WaypointMaterial.LoadSynchronous() : nullptr;
	const int32 Last = WorldPoints.Num() - 1;

	ResizePool(PathWaypoints, WorldPoints.Num(), PathSpline, this);
	for (int32 i = 0; i < WorldPoints.Num(); ++i)
	{
		UStaticMeshComponent* Wp       = PathWaypoints[i];
		UStaticMesh*        WpSlotMesh = WpMesh;
		UMaterialInterface* WpSlotMat  = WpMat;
		if (i == 0 && Config->bShowStartPoint && !Config->StartPointMesh.IsNull())
		{
			WpSlotMesh = Config->StartPointMesh.LoadSynchronous();
			WpSlotMat  = Config->StartPointMaterial.IsNull() ? nullptr : Config->StartPointMaterial.LoadSynchronous();
		}
		else if (i == Last && Config->bShowEndPoint && !Config->EndPointMesh.IsNull())
		{
			WpSlotMesh = Config->EndPointMesh.LoadSynchronous();
			WpSlotMat  = Config->EndPointMaterial.IsNull() ? nullptr : Config->EndPointMaterial.LoadSynchronous();
		}
		if (WpSlotMesh)
		{
			Wp->SetWorldLocation(WorldPoints[i]);
			Wp->SetStaticMesh(WpSlotMesh);
			if (WpSlotMat) Wp->SetMaterial(0, WpSlotMat);
			Wp->SetVisibility(true);
		}
		else
		{
			Wp->SetVisibility(false);
		}
	}
}
