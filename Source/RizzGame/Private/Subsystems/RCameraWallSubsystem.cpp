// RCameraWallSubsystem.cpp

#include "Subsystems/RCameraWallSubsystem.h"
#include "Gameplay/RCameraWall.h"
#include "Subsystems/REventBusSubsystem.h"

void URCameraWallSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UREventBusSubsystem* EventBus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			WallToggledHandle = EventBus->Subscribe<FCameraWallToggled>(
				this, &URCameraWallSubsystem::OnCameraWallToggled);
			WallGroupToggledHandle = EventBus->Subscribe<FCameraWallGroupToggled>(
				this, &URCameraWallSubsystem::OnCameraWallGroupToggled);
		}
	}
}

void URCameraWallSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UREventBusSubsystem* EventBus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			EventBus->Unsubscribe<FCameraWallToggled>(WallToggledHandle);
			EventBus->Unsubscribe<FCameraWallGroupToggled>(WallGroupToggledHandle);
		}
	}

	Super::Deinitialize();
}

void URCameraWallSubsystem::RegisterWall(ARCameraWall* Wall)
{
	if (Wall)
	{
		RegisteredWalls.AddUnique(Wall);
	}
}

void URCameraWallSubsystem::UnregisterWall(ARCameraWall* Wall)
{
	RegisteredWalls.Remove(Wall);
}

void URCameraWallSubsystem::SetWallEnabled(FName WallId, bool bEnabled)
{
	for (ARCameraWall* Wall : RegisteredWalls)
	{
		if (Wall && Wall->GetWallId() == WallId)
		{
			Wall->SetWallEnabled(bEnabled);
			break;
		}
	}
}

void URCameraWallSubsystem::SetWallGroupEnabled(FGameplayTag WallGroup, bool bEnabled)
{
	for (ARCameraWall* Wall : RegisteredWalls)
	{
		if (Wall && Wall->GetWallGroup().MatchesTagExact(WallGroup))
		{
			Wall->SetWallEnabled(bEnabled);
		}
	}
}

TArray<ARCameraWall*> URCameraWallSubsystem::GetAllWallsInGroup(FGameplayTag WallGroup) const
{
	TArray<ARCameraWall*> Result;
	for (ARCameraWall* Wall : RegisteredWalls)
	{
		if (Wall && Wall->GetWallGroup().MatchesTagExact(WallGroup))
		{
			Result.Add(Wall);
		}
	}
	return Result;
}

bool URCameraWallSubsystem::IsWallGroupEnabled(FGameplayTag WallGroup) const
{
	for (const ARCameraWall* Wall : RegisteredWalls)
	{
		if (Wall && Wall->GetWallGroup().MatchesTagExact(WallGroup) && Wall->IsWallEnabled())
		{
			return true;
		}
	}
	return false;
}

FVector URCameraWallSubsystem::ClampPositionToWalls(const FVector& OldPos, const FVector& NewPos) const
{
	return ClampPositionInternal(OldPos, NewPos, false);
}

FVector URCameraWallSubsystem::ClampCameraActorToWalls(const FVector& OldCameraPos, const FVector& NewCameraPos) const
{
	return ClampPositionInternal(OldCameraPos, NewCameraPos, true);
}

FVector URCameraWallSubsystem::ClampPositionInternal(const FVector& OldPos, const FVector& NewPos, bool bCameraActorOnly) const
{
	if (RegisteredWalls.IsEmpty())
	{
		return NewPos;
	}

	constexpr int32 MaxIterations = 3;
	constexpr float Epsilon = 5.0f;
	constexpr float PushOffset = 1.0f;

	const FVector2D OldPos2D(OldPos.X, OldPos.Y);
	FVector CorrectedPos = NewPos;

	for (int32 Iter = 0; Iter < MaxIterations; ++Iter)
	{
		bool bCorrected = false;

		for (const ARCameraWall* Wall : RegisteredWalls)
		{
			if (!Wall || !Wall->IsWallEnabled())
			{
				continue;
			}

			// When clamping the camera actor, only consider walls that have this feature enabled
			if (bCameraActorOnly && !Wall->BlocksCameraActor())
			{
				continue;
			}

			const bool bIsTwoSided = Wall->IsTwoSided();

			for (const FWallSegment2D& Seg : Wall->GetSegments())
			{
				if (Seg.ABLenSq < KINDA_SMALL_NUMBER)
				{
					continue;
				}

				const FVector2D Pos2D(CorrectedPos.X, CorrectedPos.Y);
				const float T = FVector2D::DotProduct(Pos2D - Seg.A, Seg.AB) / Seg.ABLenSq;

				const float EpsilonT = Epsilon * FMath::InvSqrt(Seg.ABLenSq);
				if (T < -EpsilonT || T > 1.0f + EpsilonT)
				{
					continue;
				}

				const float SignedDist = FVector2D::DotProduct(Pos2D - Seg.A, Seg.Normal);
				const float OldSignedDist = FVector2D::DotProduct(OldPos2D - Seg.A, Seg.Normal);

				// Crossing from normal side (positive → negative)
				if (OldSignedDist >= 0.0f && SignedDist < 0.0f)
				{
					CorrectedPos.X += Seg.Normal.X * (-SignedDist + PushOffset);
					CorrectedPos.Y += Seg.Normal.Y * (-SignedDist + PushOffset);
					bCorrected = true;
				}
				// Two-sided: also block crossing from the back side (negative → positive)
				else if (bIsTwoSided && OldSignedDist <= 0.0f && SignedDist > 0.0f)
				{
					CorrectedPos.X -= Seg.Normal.X * (SignedDist + PushOffset);
					CorrectedPos.Y -= Seg.Normal.Y * (SignedDist + PushOffset);
					bCorrected = true;
				}
			}
		}

		if (!bCorrected)
		{
			break;
		}
	}

	return CorrectedPos;
}

void URCameraWallSubsystem::OnCameraWallToggled(const FCameraWallToggled& Event)
{
	SetWallEnabled(Event.WallId, Event.bEnabled);
}

void URCameraWallSubsystem::OnCameraWallGroupToggled(const FCameraWallGroupToggled& Event)
{
	SetWallGroupEnabled(Event.WallGroup, Event.bEnabled);
}
