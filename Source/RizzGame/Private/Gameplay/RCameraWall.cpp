// RCameraWall.cpp

#include "Gameplay/RCameraWall.h"
#include "Subsystems/RCameraWallSubsystem.h"
#include "Components/BillboardComponent.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "NativeGameplayTags.h"

// --- Native Gameplay Tags ---
UE_DEFINE_GAMEPLAY_TAG(TAG_CameraWall_Zone1, "CameraWall.Zone1");
UE_DEFINE_GAMEPLAY_TAG(TAG_CameraWall_Zone2, "CameraWall.Zone2");
UE_DEFINE_GAMEPLAY_TAG(TAG_CameraWall_Combat, "CameraWall.Combat");

ARCameraWall::ARCameraWall()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	WallSpline = CreateDefaultSubobject<USplineComponent>(TEXT("WallSpline"));
	WallSpline->SetupAttachment(Root);
	WallSpline->ClearSplinePoints();
	WallSpline->AddSplineLocalPoint(FVector::ZeroVector);
	WallSpline->AddSplineLocalPoint(FVector(500.0f, 0.0f, 0.0f));
	WallSpline->SetSplinePointType(0, ESplinePointType::Linear);
	WallSpline->SetSplinePointType(1, ESplinePointType::Linear);

#if WITH_EDITORONLY_DATA
	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(Root);
#endif
}

void ARCameraWall::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Force all spline points to linear so we get straight wall segments
	if (WallSpline)
	{
		const int32 NumPoints = WallSpline->GetNumberOfSplinePoints();
		for (int32 i = 0; i < NumPoints; ++i)
		{
			WallSpline->SetSplinePointType(i, ESplinePointType::Linear, false);
		}
		WallSpline->UpdateSpline();
	}

	RebuildSegments();
}

void ARCameraWall::BeginPlay()
{
	Super::BeginPlay();

	bEnabled = bStartEnabled;
	RebuildSegments();
	CachedSubsystem = GetWorld()->GetSubsystem<URCameraWallSubsystem>();

	if (CachedSubsystem)
	{
		CachedSubsystem->RegisterWall(this);
	}
}

void ARCameraWall::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedSubsystem)
	{
		CachedSubsystem->UnregisterWall(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ARCameraWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Rebuild segments every frame in editor so spline edits are reflected instantly
	if (!GetWorld()->IsGameWorld())
	{
		RebuildSegments();
	}

	if (bShowDebug)
	{
		DrawDebugWall();
	}
}

void ARCameraWall::SetWallEnabled(bool bInEnabled)
{
	bEnabled = bInEnabled;
}

void ARCameraWall::RebuildSegments()
{
	if (!WallSpline)
	{
		CachedSegments.SetNum(0, EAllowShrinking::No);
		return;
	}

	const int32 NumPoints = WallSpline->GetNumberOfSplinePoints();
	if (NumPoints < 2)
	{
		CachedSegments.SetNum(0, EAllowShrinking::No);
		return;
	}

	const int32 NumSegments = NumPoints - 1;
	CachedSegments.SetNum(NumSegments, EAllowShrinking::No);

	FVector PrevWorldPoint = WallSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

	for (int32 i = 0; i < NumSegments; ++i)
	{
		const FVector NextWorldPoint = WallSpline->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World);

		FWallSegment2D& Seg = CachedSegments[i];
		Seg.A = FVector2D(PrevWorldPoint.X, PrevWorldPoint.Y);
		Seg.B = FVector2D(NextWorldPoint.X, NextWorldPoint.Y);
		Seg.AB = Seg.B - Seg.A;
		Seg.ABLenSq = Seg.AB.SizeSquared();

		const FVector2D Dir = Seg.AB.GetSafeNormal();
		Seg.Normal = bFlipNormal ? FVector2D(Dir.Y, -Dir.X) : FVector2D(-Dir.Y, Dir.X);

		PrevWorldPoint = NextWorldPoint;
	}
}

void ARCameraWall::DrawDebugWall() const
{
#if ENABLE_DRAW_DEBUG
	if (!WallSpline || CachedSegments.Num() == 0)
	{
		return;
	}

	const FColor GroupCol = GetGroupColor();
	const FColor LineColor = bEnabled ? GroupCol : FColor(GroupCol.R / 3, GroupCol.G / 3, GroupCol.B / 3);
	const float Thickness = bEnabled ? 3.0f : 1.0f;
	const float HalfHeight = WallHeight * 0.5f;

	const float ActorZ = GetActorLocation().Z;

	for (int32 i = 0; i < CachedSegments.Num(); ++i)
	{
		const FWallSegment2D& Seg = CachedSegments[i];

		const FVector A3D(Seg.A.X, Seg.A.Y, ActorZ);
		const FVector B3D(Seg.B.X, Seg.B.Y, ActorZ);

		// Bottom line
		DrawDebugLine(GetWorld(), A3D - FVector::UpVector * HalfHeight, B3D - FVector::UpVector * HalfHeight, LineColor, false, -1.0f, 0, Thickness);
		// Top line
		DrawDebugLine(GetWorld(), A3D + FVector::UpVector * HalfHeight, B3D + FVector::UpVector * HalfHeight, LineColor, false, -1.0f, 0, Thickness);
		// Vertical edges
		DrawDebugLine(GetWorld(), A3D - FVector::UpVector * HalfHeight, A3D + FVector::UpVector * HalfHeight, LineColor, false, -1.0f, 0, Thickness);
		DrawDebugLine(GetWorld(), B3D - FVector::UpVector * HalfHeight, B3D + FVector::UpVector * HalfHeight, LineColor, false, -1.0f, 0, Thickness);

		// Normal arrow at midpoint of each segment
		const FVector MidPoint = (A3D + B3D) * 0.5f;
		const FVector ArrowEnd = MidPoint + FVector(Seg.Normal.X, Seg.Normal.Y, 0.0f) * 100.0f;
		DrawDebugDirectionalArrow(GetWorld(), MidPoint, ArrowEnd, 40.0f, LineColor, false, -1.0f, 0, 2.0f);
	}
#endif
}

FColor ARCameraWall::GetGroupColor() const
{
	if (!WallGroup.IsValid())
	{
		return FColor::Green;
	}

	const uint32 Hash = GetTypeHash(WallGroup.GetTagName());
	const float Hue = (Hash % 360);
	return FLinearColor::MakeFromHSV8(static_cast<uint8>(Hue * 255 / 360), 200, 230).ToFColor(true);
}
