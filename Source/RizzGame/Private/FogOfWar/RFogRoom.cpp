// RFogRoom.cpp
#include "FogOfWar/RFogRoom.h"
#include "EngineUtils.h"
#include "GameFramework/Info.h"
#include "GameFramework/Controller.h"
#include "Engine/LevelScriptActor.h"
DEFINE_LOG_CATEGORY_STATIC(LogFogOfWarRoom, Log, All);
#include "Subsystems/RFogOfWarSubsystem.h"
#include "Components/SplineComponent.h"
#include "Components/BillboardComponent.h"
#include "DrawDebugHelpers.h"

ARFogRoom::ARFogRoom()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	RoomSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RoomSpline"));
	RoomSpline->SetupAttachment(SceneRoot);
	RoomSpline->SetClosedLoop(true);
	RoomSpline->ClearSplinePoints();

	const float HalfSize = 200.0f;
	RoomSpline->AddSplineLocalPoint(FVector(-HalfSize, -HalfSize, 0.0f));
	RoomSpline->AddSplineLocalPoint(FVector( HalfSize, -HalfSize, 0.0f));
	RoomSpline->AddSplineLocalPoint(FVector( HalfSize,  HalfSize, 0.0f));
	RoomSpline->AddSplineLocalPoint(FVector(-HalfSize,  HalfSize, 0.0f));

	for (int32 i = 0; i < RoomSpline->GetNumberOfSplinePoints(); ++i)
	{
		RoomSpline->SetSplinePointType(i, ESplinePointType::Linear);
	}

#if WITH_EDITORONLY_DATA
	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(SceneRoot);
#endif
}

void ARFogRoom::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (RoomSpline)
	{
		const int32 NumPoints = RoomSpline->GetNumberOfSplinePoints();
		for (int32 i = 0; i < NumPoints; ++i)
		{
			RoomSpline->SetSplinePointType(i, ESplinePointType::Linear, false);
		}
		RoomSpline->UpdateSpline();
	}

	RebuildPolygon();

	// Editor only — runtime detection runs in BeginPlay when all actors are ready
	if (!GetWorld() || !GetWorld()->IsGameWorld())
	{
		AutoDetectContainedActors();
	}
}

void ARFogRoom::RefreshContainedActors()
{
	RebuildPolygon();
	AutoDetectContainedActors();
}

void ARFogRoom::BeginPlay()
{
	Super::BeginPlay();

	RebuildPolygon();
	AutoDetectContainedActors();

	CachedSubsystem = GetWorld()->GetSubsystem<URFogOfWarSubsystem>();
	if (CachedSubsystem)
	{
		CachedSubsystem->RegisterRoom(this);
	}
	else
	{
		UE_LOG(LogFogOfWarRoom, Warning, TEXT("Room '%s': URFogOfWarSubsystem not found"), *RoomId.ToString());
	}

	if (!bStartRevealed)
	{
		for (AActor* Actor : ContainedActors)
		{
			if (Actor)
			{
				Actor->SetActorHiddenInGame(true);
				Actor->SetActorEnableCollision(false);
			}
		}
		UE_LOG(LogFogOfWarRoom, Log, TEXT("Room '%s' starts hidden | %d actor(s) concealed"),
			*RoomId.ToString(), ContainedActors.Num());
	}
	else
	{
		bRevealed = true;
		UE_LOG(LogFogOfWarRoom, Log, TEXT("Room '%s' starts revealed"), *RoomId.ToString());
	}
}

void ARFogRoom::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedSubsystem)
	{
		CachedSubsystem->UnregisterRoom(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ARFogRoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetWorld()->IsGameWorld())
	{
		RebuildPolygon();
	}

	if (bShowDebug)
	{
		DrawDebugRoom();
	}
}

void ARFogRoom::SetRevealed(bool bInRevealed)
{
	bRevealed = bInRevealed;

	for (AActor* Actor : ContainedActors)
	{
		if (Actor)
		{
			Actor->SetActorHiddenInGame(!bRevealed);
			Actor->SetActorEnableCollision(bRevealed);
		}
	}
}

bool ARFogRoom::ContainsPoint2D(const FVector2D& Point) const
{
	if (CachedPolygon.Num() < 3)
	{
		return false;
	}

	int32 CrossingCount = 0;
	const int32 NumPoints = CachedPolygon.Num();

	for (int32 i = 0; i < NumPoints; ++i)
	{
		const FVector2D& A = CachedPolygon[i];
		const FVector2D& B = CachedPolygon[(i + 1) % NumPoints];

		if (((A.Y <= Point.Y) && (B.Y > Point.Y)) || ((B.Y <= Point.Y) && (A.Y > Point.Y)))
		{
			const float T = (Point.Y - A.Y) / (B.Y - A.Y);
			if (Point.X < A.X + T * (B.X - A.X))
			{
				CrossingCount++;
			}
		}
	}

	return (CrossingCount % 2) != 0;
}

void ARFogRoom::GetOverlappingGridCells(const FRFogGrid& InGrid, TArray<FIntPoint>& OutCells) const
{
	if (CachedPolygon.IsEmpty())
	{
		return;
	}

	FBox2D Bounds(EForceInit::ForceInit);
	for (const FVector2D& Point : CachedPolygon)
	{
		Bounds += Point;
	}

	const FIntPoint MinCell = InGrid.WorldToCell(FVector(Bounds.Min.X, Bounds.Min.Y, 0.0f));
	const FIntPoint MaxCell = InGrid.WorldToCell(FVector(Bounds.Max.X, Bounds.Max.Y, 0.0f));

	for (int32 Y = MinCell.Y; Y <= MaxCell.Y; ++Y)
	{
		for (int32 X = MinCell.X; X <= MaxCell.X; ++X)
		{
			if (!InGrid.IsValidCell(X, Y))
			{
				continue;
			}

			const FVector2D CellCenter = InGrid.CellToWorld(X, Y);
			if (ContainsPoint2D(CellCenter))
			{
				OutCells.Add(FIntPoint(X, Y));
			}
		}
	}
}

void ARFogRoom::AutoDetectContainedActors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ContainedActors.Reset();

	const FVector RoomOrigin  = GetActorLocation();
	const float   HalfHeight  = RoomHeight * 0.5f;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor == this)
		{
			continue;
		}

		// Skip pure-logic system actors (no visual presence in the level)
		if (Actor->IsA<ARFogRoom>())          { continue; }
		if (Actor->IsA<AInfo>())              { continue; }
		if (Actor->IsA<AController>())        { continue; }
		if (Actor->IsA<ALevelScriptActor>())  { continue; }

		const FVector Pos = Actor->GetActorLocation();

		if (FMath::Abs(Pos.Z - RoomOrigin.Z) > HalfHeight)
		{
			continue;
		}

		if (ContainsPoint2D(FVector2D(Pos.X, Pos.Y)))
		{
			ContainedActors.Add(Actor);
		}
	}

	UE_LOG(LogFogOfWarRoom, Log, TEXT("Room '%s': auto-detected %d contained actors"),
		*RoomId.ToString(), ContainedActors.Num());
}

void ARFogRoom::RebuildPolygon()
{
	if (!RoomSpline)
	{
		CachedPolygon.Reset();
		return;
	}

	const int32 NumPoints = RoomSpline->GetNumberOfSplinePoints();
	CachedPolygon.SetNum(NumPoints, EAllowShrinking::No);

	for (int32 i = 0; i < NumPoints; ++i)
	{
		const FVector WorldPoint = RoomSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		CachedPolygon[i] = FVector2D(WorldPoint.X, WorldPoint.Y);
	}
}

void ARFogRoom::DrawDebugRoom() const
{
#if ENABLE_DRAW_DEBUG
	if (CachedPolygon.Num() < 2)
	{
		return;
	}

	UWorld* World         = GetWorld();
	const FColor RoomColor = GetGroupColor();
	const float ActorZ    = GetActorLocation().Z;
	const int32 NumPoints = CachedPolygon.Num();

	const FColor OutlineColor = bRevealed
		? RoomColor
		: FColor(RoomColor.R / 3, RoomColor.G / 3, RoomColor.B / 3);
	const float OutlineThickness = bRevealed ? 3.0f : 1.0f;

	FVector2D PolygonCenter = FVector2D::ZeroVector;
	for (int32 i = 0; i < NumPoints; ++i)
	{
		const FVector2D& A = CachedPolygon[i];
		const FVector2D& B = CachedPolygon[(i + 1) % NumPoints];

		DrawDebugLine(World,
			FVector(A.X, A.Y, ActorZ),
			FVector(B.X, B.Y, ActorZ),
			OutlineColor, false, -1.0f, 0, OutlineThickness);

		PolygonCenter += A;
	}
	PolygonCenter /= static_cast<float>(NumPoints);

	const FString Label = FString::Printf(TEXT("[%s]\n%s"),
		*RoomId.ToString(),
		RoomGroup.IsValid() ? *RoomGroup.ToString() : TEXT("no group"));
	DrawDebugString(World,
		FVector(PolygonCenter.X, PolygonCenter.Y, ActorZ + 50.0f),
		Label, nullptr, OutlineColor, -1.0f, false, 1.0f);

	const FVector RoomCenter3D(PolygonCenter.X, PolygonCenter.Y, ActorZ);

	for (const AActor* Actor : ContainedActors)
	{
		if (!Actor)
		{
			continue;
		}

		const FVector ActorPos = Actor->GetActorLocation();

		const FColor ActorColor = bRevealed ? FColor::Green : FColor::Red;

		DrawDebugLine(World, RoomCenter3D, ActorPos, ActorColor, false, -1.0f, 0, 1.0f);
		DrawDebugSphere(World, ActorPos, 20.0f, 8, ActorColor, false, -1.0f, 0, 1.5f);
		DrawDebugString(World, ActorPos + FVector(0, 0, 30.0f),
			Actor->GetName(), nullptr, ActorColor, -1.0f, false, 0.8f);
	}
#endif
}

FColor ARFogRoom::GetGroupColor() const
{
	if (!RoomGroup.IsValid())
	{
		return FColor::Cyan;
	}

	const uint32 Hash = GetTypeHash(RoomGroup.GetTagName());
	const float Hue = static_cast<float>(Hash % 360);
	return FLinearColor::MakeFromHSV8(static_cast<uint8>(Hue * 255 / 360), 200, 230).ToFColor(true);
}
