// RFogOfWarSubsystem.cpp
#include "Subsystems/RFogOfWarSubsystem.h"
#include "FogOfWar/RFogRoom.h"
#include "Subsystems/RPartySubsystem.h"
#include "Subsystems/REventBusSubsystem.h"
#include "Core/RCharacterBase.h"
#include "Events/FogOfWarEvents.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Settings/RFogOfWarSettings.h"
#include "Engine/Texture2D.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "PixelFormat.h"
#include "RHI.h"

DEFINE_LOG_CATEGORY_STATIC(LogFogOfWar, Log, All);

void URFogOfWarSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void URFogOfWarSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LOSTimerHandle);
	}

	Super::Deinitialize();
}

void URFogOfWarSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (const URFogOfWarSettings* Settings = URFogOfWarSettings::Get())
	{
		CellSize               = Settings->CellSize;
		DefaultVisionRadius    = Settings->DefaultVisionRadius;
		LOSUpdateInterval      = Settings->LOSUpdateInterval;
		LOSTraceChannel        = Settings->LOSTraceChannel;
		bAutoComputeGridBounds = Settings->bAutoComputeGridBounds;
		GridExtentMin          = Settings->GridExtentMin;
		GridExtentMax          = Settings->GridExtentMax;
		bDrawGrid              = Settings->bDrawGrid;
		DrawGridZ              = Settings->DrawGridZ;
		bVisualFogEnabled      = Settings->bEnableVisualFog;
		FogTemporalSpeed       = Settings->FogTemporalSpeed;
	}

	InitializeGrid();

	UE_LOG(LogFogOfWar, Log, TEXT("Grid initialized: %dx%d cells | origin=(%.0f, %.0f) | cellSize=%.0f | totalCells=%d"),
		Grid.GridWidth, Grid.GridHeight,
		Grid.WorldOrigin.X, Grid.WorldOrigin.Y,
		Grid.CellSize,
		Grid.GridWidth * Grid.GridHeight);

	InitializeVisualFog();

	GetWorld()->GetTimerManager().SetTimer(
		LOSTimerHandle,
		this,
		&URFogOfWarSubsystem::UpdateLOS,
		LOSUpdateInterval,
		true,
		LOSUpdateInterval
	);
}

void URFogOfWarSubsystem::RegisterRoom(ARFogRoom* Room)
{
	if (Room)
	{
		RegisteredRooms.AddUnique(Room);
		UE_LOG(LogFogOfWar, Log, TEXT("Room registered: '%s' | group=%s | totalRooms=%d"),
			*Room->GetRoomId().ToString(),
			*Room->GetRoomGroup().ToString(),
			RegisteredRooms.Num());
	}
}

void URFogOfWarSubsystem::UnregisterRoom(ARFogRoom* Room)
{
	if (Room)
	{
		UE_LOG(LogFogOfWar, Log, TEXT("Room unregistered: '%s'"), *Room->GetRoomId().ToString());
	}
	RegisteredRooms.Remove(Room);
}

void URFogOfWarSubsystem::RevealRoom(ARFogRoom* Room)
{
	if (!Room || Room->IsRevealed())
	{
		return;
	}

	Room->SetRevealed(true);
	MarkRoomCellsExplored(Room);

	UE_LOG(LogFogOfWar, Log, TEXT("Room revealed: '%s' | group=%s | containedActors=%d"),
		*Room->GetRoomId().ToString(),
		*Room->GetRoomGroup().ToString(),
		Room->ContainedActors.Num());

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UREventBusSubsystem* EventBus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			FRRoomRevealed Event;
			Event.RoomId    = Room->GetRoomId();
			Event.RoomGroup = Room->GetRoomGroup();
			EventBus->Publish(Event);
		}
	}
}

void URFogOfWarSubsystem::HideRoom(ARFogRoom* Room)
{
	if (!Room || !Room->IsRevealed())
	{
		return;
	}

	Room->SetRevealed(false);

	UE_LOG(LogFogOfWar, Log, TEXT("Room hidden: '%s' | group=%s"),
		*Room->GetRoomId().ToString(),
		*Room->GetRoomGroup().ToString());

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UREventBusSubsystem* EventBus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			FRRoomHidden Event;
			Event.RoomId    = Room->GetRoomId();
			Event.RoomGroup = Room->GetRoomGroup();
			EventBus->Publish(Event);
		}
	}
}

void URFogOfWarSubsystem::SetRoomVisible(FGameplayTag RoomId, bool bVisible)
{
	for (ARFogRoom* Room : RegisteredRooms)
	{
		if (Room && Room->GetRoomId().MatchesTagExact(RoomId))
		{
			if (bVisible) { RevealRoom(Room); } else { HideRoom(Room); }
			return;
		}
	}
	UE_LOG(LogFogOfWar, Warning, TEXT("SetRoomVisible: no room found with id '%s'"), *RoomId.ToString());
}

void URFogOfWarSubsystem::RevealRoomById(FGameplayTag RoomId)
{
	SetRoomVisible(RoomId, true);
}

void URFogOfWarSubsystem::RevealRoomGroup(FGameplayTag RoomGroup)
{
	for (ARFogRoom* Room : RegisteredRooms)
	{
		if (Room && Room->GetRoomGroup().MatchesTagExact(RoomGroup))
		{
			RevealRoom(Room);
		}
	}
}
void URFogOfWarSubsystem::HideRoomById(FGameplayTag RoomId)
{
	SetRoomVisible(RoomId, false);
}

void URFogOfWarSubsystem::HideRoomGroup(FGameplayTag RoomGroup)
{
	for (ARFogRoom* Room : RegisteredRooms)
	{
		if (Room && Room->GetRoomGroup().MatchesTagExact(RoomGroup))
		{
			HideRoom(Room);
		}
	}
}

void URFogOfWarSubsystem::SetRoomGroupRevealed(FGameplayTag RoomGroup, bool bReveal)
{
	if (bReveal)
	{
		RevealRoomGroup(RoomGroup);
	}
	else
	{
		HideRoomGroup(RoomGroup);
	}
}

bool URFogOfWarSubsystem::IsRoomGroupRevealed(FGameplayTag RoomGroup) const
{
	for (const ARFogRoom* Room : RegisteredRooms)
	{
		if (Room && Room->GetRoomGroup().MatchesTagExact(RoomGroup) && Room->IsRevealed())
		{
			return true;
		}
	}
	return false;
}

TArray<ARFogRoom*> URFogOfWarSubsystem::GetAllRoomsInGroup(FGameplayTag RoomGroup) const
{
	TArray<ARFogRoom*> Result;
	for (ARFogRoom* Room : RegisteredRooms)
	{
		if (Room && Room->GetRoomGroup().MatchesTagExact(RoomGroup))
		{
			Result.Add(Room);
		}
	}
	return Result;
}

ERFogState URFogOfWarSubsystem::GetFogStateAtWorldPos(const FVector& WorldPos) const
{
	const FIntPoint Cell = Grid.WorldToCell(WorldPos);
	return Grid.GetState(Cell.X, Cell.Y);
}

bool URFogOfWarSubsystem::IsLocationVisible(const FVector& WorldPos) const
{
	return GetFogStateAtWorldPos(WorldPos) == ERFogState::Visible;
}

bool URFogOfWarSubsystem::IsLocationExplored(const FVector& WorldPos) const
{
	const ERFogState State = GetFogStateAtWorldPos(WorldPos);
	return State == ERFogState::Explored || State == ERFogState::Visible;
}

TMap<FGameplayTag, bool> URFogOfWarSubsystem::GetRoomRevealStates() const
{
	TMap<FGameplayTag, bool> Result;
	for (const ARFogRoom* Room : RegisteredRooms)
	{
		if (Room)
		{
			Result.Add(Room->GetRoomId(), Room->IsRevealed());
		}
	}
	return Result;
}

void URFogOfWarSubsystem::RestoreGrid(const FRFogGrid& SavedGrid)
{
	Grid = SavedGrid;
}

void URFogOfWarSubsystem::RestoreRoomStates(const TMap<FGameplayTag, bool>& SavedStates)
{
	for (ARFogRoom* Room : RegisteredRooms)
	{
		if (!Room)
		{
			continue;
		}

		const bool* bShouldReveal = SavedStates.Find(Room->GetRoomId());
		if (bShouldReveal)
		{
			Room->SetRevealed(*bShouldReveal);
		}
	}
}

void URFogOfWarSubsystem::InitializeGrid()
{
	FVector2D Min;
	FVector2D Max;

	if (bAutoComputeGridBounds && RegisteredRooms.Num() > 0)
	{
		FBox2D Bounds(EForceInit::ForceInit);
		for (const ARFogRoom* Room : RegisteredRooms)
		{
			if (!Room)
			{
				continue;
			}
			for (const FVector2D& Point : Room->GetPolygonPoints())
			{
				Bounds += Point;
			}
		}

		const float Padding = CellSize * 4.0f;
		Min = Bounds.Min - FVector2D(Padding);
		Max = Bounds.Max + FVector2D(Padding);
	}
	else
	{
		Min = GridExtentMin;
		Max = GridExtentMax;
	}

	const FVector2D Size = Max - Min;
	const int32 Width    = FMath::Max(1, FMath::CeilToInt(Size.X / CellSize));
	const int32 Height   = FMath::Max(1, FMath::CeilToInt(Size.Y / CellSize));

	Grid.Initialize(Min, CellSize, Width, Height);
}

void URFogOfWarSubsystem::UpdateLOS()
{
	if (Grid.Cells.IsEmpty())
	{
		return;
	}

	Grid.DemoteAllVisible();

	const TArray<FVector> Sources = GatherVisionSources();
	if (Sources.IsEmpty())
	{
		return;
	}

	for (const FVector& Pos : Sources)
	{
		ComputeLOSForCharacter(Pos, DefaultVisionRadius);
	}

	int32 CellsChanged = 0;
	for (const uint8 Cell : Grid.Cells)
	{
		if (static_cast<ERFogState>(Cell) == ERFogState::Visible)
		{
			CellsChanged++;
		}
	}

	UE_LOG(LogFogOfWar, Verbose, TEXT("LOS update: %d visible cells | %d sources"),
		CellsChanged, Sources.Num());

	if (bDrawGrid)
	{
		DrawGridDebug();
	}

	if (CellsChanged > 0)
	{
		if (UGameInstance* GI = GetWorld()->GetGameInstance())
		{
			if (UREventBusSubsystem* EventBus = GI->GetSubsystem<UREventBusSubsystem>())
			{
				FRFogGridUpdated Event;
				Event.CellsChanged = CellsChanged;
				EventBus->Publish(Event);
			}
		}
	}

	PublishGridToTexture(LOSUpdateInterval);
}

void URFogOfWarSubsystem::ComputeLOSForCharacter(const FVector& CharacterPos, float VisionRadius)
{
	const FIntPoint Center = Grid.WorldToCell(CharacterPos);
	const int32 RadiusCells = FMath::CeilToInt(VisionRadius / Grid.CellSize);

	const int32 MinX = FMath::Max(0, Center.X - RadiusCells);
	const int32 MaxX = FMath::Min(Grid.GridWidth - 1, Center.X + RadiusCells);
	const int32 MinY = FMath::Max(0, Center.Y - RadiusCells);
	const int32 MaxY = FMath::Min(Grid.GridHeight - 1, Center.Y + RadiusCells);

	const float RadiusSq = FMath::Square(VisionRadius);

	for (int32 Y = MinY; Y <= MaxY; ++Y)
	{
		for (int32 X = MinX; X <= MaxX; ++X)
		{
			const FVector2D CellCenter = Grid.CellToWorld(X, Y);

			const float DX = CellCenter.X - CharacterPos.X;
			const float DY = CellCenter.Y - CharacterPos.Y;
			if (DX * DX + DY * DY > RadiusSq)
			{
				continue;
			}

			if (TraceToCell(CharacterPos, CellCenter))
			{
				Grid.MarkCellVisible(X, Y);
			}
		}
	}
}

bool URFogOfWarSubsystem::TraceToCell(const FVector& From, const FVector2D& CellCenter) const
{
	const FVector To(CellCenter.X, CellCenter.Y, From.Z);
	return !GetWorld()->LineTraceTestByChannel(From, To, LOSTraceChannel);
}

TArray<FVector> URFogOfWarSubsystem::GatherVisionSources() const
{
	TArray<FVector> Sources;

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		return Sources;
	}

	URPartySubsystem* PartySubsystem = GI->GetSubsystem<URPartySubsystem>();
	if (!PartySubsystem)
	{
		return Sources;
	}

	for (const ARCharacterBase* Member : PartySubsystem->PartyMembers)
	{
		if (Member)
		{
			Sources.Add(Member->GetActorLocation());
		}
	}

	return Sources;
}

void URFogOfWarSubsystem::MarkRoomCellsExplored(ARFogRoom* Room)
{
	if (!Room)
	{
		return;
	}

	TArray<FIntPoint> Cells;
	Room->GetOverlappingGridCells(Grid, Cells);

	int32 MarkedCount = 0;
	for (const FIntPoint& Cell : Cells)
	{
		if (Grid.GetState(Cell.X, Cell.Y) == ERFogState::Unknown)
		{
			Grid.SetState(Cell.X, Cell.Y, ERFogState::Explored);
			MarkedCount++;
		}
	}

	UE_LOG(LogFogOfWar, Log, TEXT("Room '%s' cells explored: %d new | %d total overlap"),
		*Room->GetRoomId().ToString(), MarkedCount, Cells.Num());
}

void URFogOfWarSubsystem::DrawGridDebug() const
{
#if ENABLE_DRAW_DEBUG
	UWorld* World = GetWorld();
	if (!World || Grid.Cells.IsEmpty())
	{
		return;
	}

	const float HalfCell    = Grid.CellSize * 0.5f;
	const float Duration    = LOSUpdateInterval * 1.5f;
	const FVector BoxExtent = FVector(HalfCell - 2.0f, HalfCell - 2.0f, 5.0f);

	for (int32 Y = 0; Y < Grid.GridHeight; ++Y)
	{
		for (int32 X = 0; X < Grid.GridWidth; ++X)
		{
			const ERFogState State = Grid.GetState(X, Y);

			FColor Color;
			switch (State)
			{
				case ERFogState::Visible:  Color = FColor::Green;  break;
				case ERFogState::Explored: Color = FColor::Yellow; break;
				default: continue; // Skip Unknown cells — no need to draw them
			}

			const FVector2D Center2D = Grid.CellToWorld(X, Y);
			const FVector Center3D(Center2D.X, Center2D.Y, DrawGridZ);

			DrawDebugBox(World, Center3D, BoxExtent, Color, false, Duration, 0, 1.0f);
		}
	}
#endif
}

void URFogOfWarSubsystem::InitializeVisualFog()
{
	if (!bVisualFogEnabled)
	{
		return;
	}

	const URFogOfWarSettings* Settings = URFogOfWarSettings::Get();
	if (!Settings)
	{
		return;
	}

	UMaterialInterface* PPMAsset = Cast<UMaterialInterface>(Settings->FogPostProcessMaterial.TryLoad());
	if (!PPMAsset)
	{
		UE_LOG(LogFogOfWar, Warning, TEXT("Visual fog disabled: FogPostProcessMaterial not set in Project Settings → Rizz → Fog Of War → Visual Fog."));
		return;
	}

	const int32 W = Grid.GridWidth;
	const int32 H = Grid.GridHeight;
	if (W <= 0 || H <= 0)
	{
		UE_LOG(LogFogOfWar, Warning, TEXT("Visual fog skipped: empty grid (%dx%d)."), W, H);
		return;
	}

	FogTexture = UTexture2D::CreateTransient(W, H, PF_G8);
	if (!FogTexture)
	{
		UE_LOG(LogFogOfWar, Error, TEXT("Visual fog: CreateTransient failed."));
		return;
	}
	FogTexture->Filter            = TF_Bilinear;
	FogTexture->AddressX          = TA_Clamp;
	FogTexture->AddressY          = TA_Clamp;
	FogTexture->MipGenSettings    = TMGS_NoMipmaps;
	FogTexture->SRGB              = false;
	FogTexture->CompressionSettings = TC_Grayscale;
	FogTexture->NeverStream       = true;
	FogTexture->UpdateResource();

	FogPPMID = UMaterialInstanceDynamic::Create(PPMAsset, this);
	if (FogPPMID)
	{
		FogPPMID->SetTextureParameterValue(FName(TEXT("FogTexture")), FogTexture);
	}

	FogMPC = Cast<UMaterialParameterCollection>(Settings->FogParameterCollection.TryLoad());
	if (FogMPC)
	{
		if (UMaterialParameterCollectionInstance* MPCI = GetWorld()->GetParameterCollectionInstance(FogMPC))
		{
			const FLinearColor Origin(Grid.WorldOrigin.X, Grid.WorldOrigin.Y, 0.0f, 0.0f);
			const float InvW = 1.0f / (Grid.CellSize * static_cast<float>(W));
			const float InvH = 1.0f / (Grid.CellSize * static_cast<float>(H));
			const FLinearColor InvSize(InvW, InvH, 0.0f, 0.0f);

			MPCI->SetVectorParameterValue(FName(TEXT("FogGridOrigin")), Origin);
			MPCI->SetVectorParameterValue(FName(TEXT("FogGridInvSize")), InvSize);
		}
	}
	else
	{
		UE_LOG(LogFogOfWar, Warning, TEXT("Visual fog: FogParameterCollection not set — material will use defaults for grid origin/size."));
	}

	SmoothedCells.Init(0.0f, W * H);
	PixelScratch.SetNumZeroed(W * H);

	UE_LOG(LogFogOfWar, Log, TEXT("Visual fog initialized: %dx%d R8 texture, DMI=%s, MPC=%s"),
		W, H,
		FogPPMID ? TEXT("ok") : TEXT("FAILED"),
		FogMPC   ? TEXT("ok") : TEXT("none"));
}

void URFogOfWarSubsystem::SpawnLevelFogVolume()
{
	if (!bVisualFogEnabled || !FogPPMID || FogPPVolume)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags = RF_Transient;

	FogPPVolume = World->SpawnActor<APostProcessVolume>(
		APostProcessVolume::StaticClass(),
		FTransform::Identity,
		SpawnParams);

	if (!FogPPVolume)
	{
		UE_LOG(LogFogOfWar, Warning, TEXT("Visual fog: failed to spawn APostProcessVolume."));
		return;
	}

	FogPPVolume->bUnbound    = true;
	FogPPVolume->BlendWeight = 1.0f;
	FogPPVolume->Priority    = 100.0f;
	FogPPVolume->bEnabled    = true;
	FogPPVolume->Tags.Add(FName(TEXT("FogOfWar")));
	FogPPVolume->Settings.AddBlendable(FogPPMID, 1.0f);

	UE_LOG(LogFogOfWar, Log, TEXT("Visual fog post-process volume spawned (unbound, priority=100)."));
}

void URFogOfWarSubsystem::PublishGridToTexture(float DeltaSeconds)
{
	if (!bVisualFogEnabled || !FogTexture || Grid.Cells.IsEmpty())
	{
		return;
	}

	const int32 W = Grid.GridWidth;
	const int32 H = Grid.GridHeight;
	const int32 Total = W * H;
	if (SmoothedCells.Num() != Total || PixelScratch.Num() != Total)
	{
		return;
	}

	const float Alpha = (FogTemporalSpeed > 0.0f)
		? (1.0f - FMath::Exp(-FogTemporalSpeed * DeltaSeconds))
		: 1.0f;

	for (int32 i = 0; i < Total; ++i)
	{
		float Target = 0.0f;
		switch (static_cast<ERFogState>(Grid.Cells[i]))
		{
			case ERFogState::Visible:  Target = 1.0f; break;
			case ERFogState::Explored: Target = 0.5f; break;
			default:                   Target = 0.0f; break;
		}

		SmoothedCells[i] = FMath::Lerp(SmoothedCells[i], Target, Alpha);
		PixelScratch[i] = static_cast<uint8>(FMath::Clamp(SmoothedCells[i] * 255.0f, 0.0f, 255.0f));
	}

	// Copy into a heap buffer so the render thread can consume it safely;
	// freed via the cleanup lambda after the GPU upload completes.
	const SIZE_T NumBytes = static_cast<SIZE_T>(Total);
	uint8* UploadData = static_cast<uint8*>(FMemory::Malloc(NumBytes));
	FMemory::Memcpy(UploadData, PixelScratch.GetData(), NumBytes);

	FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(0, 0, 0, 0, W, H);

	FogTexture->UpdateTextureRegions(
		0,                              // MipIndex
		1,                              // NumRegions
		Region,                         // Regions
		static_cast<uint32>(W),         // SrcPitch (bytes per row)
		1,                              // SrcBpp
		UploadData,
		[](uint8* Data, const FUpdateTextureRegion2D* Regions)
		{
			FMemory::Free(Data);
			delete Regions;
		});
}

