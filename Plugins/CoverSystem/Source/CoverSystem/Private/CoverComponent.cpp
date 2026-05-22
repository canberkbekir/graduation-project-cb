


#include "CoverComponent.h"

#include "GameFramework/Character.h"

UCoverComponent::UCoverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UCoverComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CoverPoint && CoverPoint->CoverActor == GetOwner())
	{
		CoverPoint->ReleaseCover();
		SetIsInCover(false);
	}
    
	Super::EndPlay(EndPlayReason);
}

void UCoverComponent::OccupyCoverPoint(ACoverPoint* NewCoverPoint, bool bIsPhysicllyReached)
{
	if (NewCoverPoint == nullptr)
		return;
	
	if (CoverPoint && CoverPoint == NewCoverPoint && CoverPoint->IsReservedBy(GetOwner()))
	{
		CoverPoint->OccupyCover(GetOwner());
		if (bIsPhysicllyReached)
			SetIsInCover(true);
		OnCoverOccupied.Broadcast(NewCoverPoint);
	}
	/*if (CoverPoint && CoverPoint != NewCoverPoint && CoverPoint->CoverActor == GetOwner())
		CoverPoint->ReleaseCover();
    
	CoverPoint = NewCoverPoint;
	
	CoverType = CoverPoint->CoverType;
	
	CrouchPosition = ECrouchCoverPositions::Idle;
	StandPosition = EStandCoverPositions::Idle;
	
	bFindOutPositionDynamically = CoverPoint->bFindOutPositionDynamically;
	CoverPoint->OccupyCover(GetOwner());*/
}

void UCoverComponent::OccupyCoverPointWithoutReservation(ACoverPoint* NewCoverPoint, bool bIsPhysicllyReached)
{
	if (NewCoverPoint == nullptr)
		return;
	
	if (CoverPoint && CoverPoint->CoverActor == GetOwner())
		CoverPoint->ReleaseCover();
	
	CoverPoint = NewCoverPoint;
	
	CoverType = CoverPoint->CoverType;
	CrouchPosition = CoverPoint->CrouchPosition;
	StandPosition = CoverPoint->StandPosition;
	bFindOutPositionDynamically = CoverPoint->bFindOutPositionDynamically;
	
	CoverPoint->OccupyCover(GetOwner());
	if (bIsPhysicllyReached)
		SetIsInCover(true);
	OnCoverOccupied.Broadcast(NewCoverPoint);
}

void UCoverComponent::ReserveCoverPoint(ACoverPoint* NewCoverPoint)
{
	if (NewCoverPoint == nullptr)
		return;
	
	if (CoverPoint && CoverPoint->CoverActor == GetOwner())
		CoverPoint->ReleaseCover();
	
	CoverPoint = NewCoverPoint;
	CoverType = CoverPoint->CoverType;
	CrouchPosition = ECrouchCoverPositions::Idle;
	StandPosition = EStandCoverPositions::Idle;
	bFindOutPositionDynamically = CoverPoint->bFindOutPositionDynamically;
	CoverPoint->ReserveCover(GetOwner());
	OnCoverReserved.Broadcast(NewCoverPoint);
}

void UCoverComponent::ClearCoverPoint()
{
	if (CoverPoint && CoverPoint->CoverActor == GetOwner())
	{
		CoverPoint->ReleaseCover();
		OnCoverFreed.Broadcast(CoverPoint);
		SetIsInCover(false);
		CoverType = ECoverType::CrouchCover;
		CrouchPosition = ECrouchCoverPositions::Idle;
		StandPosition = EStandCoverPositions::Idle;
		CoverPoint = nullptr; 
	}
}

void UCoverComponent::SetIsInCover(bool bNewInCover)
{
	if (bIsInCover == bNewInCover)
		return;
    
	bIsInCover = bNewInCover;
    
	if (bIsInCover)
		OnEnteredCover.Broadcast(CoverPoint);
	else
		OnExitedCover.Broadcast(CoverPoint);
}

/*void UCoverComponent::SetIsInCover(bool bNewInCover)
{
	if (bIsInCover == bNewInCover)
		return;
    
	bIsInCover = bNewInCover;
    
	if (bIsInCover)
	{
		if (AActor* Owner = GetOwner())
		{
			FVector Velocity = FVector::ZeroVector;
			
			if (ACharacter* Char = Cast<ACharacter>(Owner))
				Velocity = Char->GetVelocity();
			else
				Velocity = Owner->GetVelocity();
            
			if (Velocity.SizeSquared2D() > 100.0f)
			{
				CoverFacingDirection = Velocity.GetSafeNormal2D();
			}
			else
			{
				CoverFacingDirection = Owner->GetActorForwardVector().GetSafeNormal2D();
			}
		}
        
		OnEnteredCover.Broadcast(CoverPoint);
	}
	else
	{
		OnExitedCover.Broadcast(CoverPoint);
		CoverFacingDirection = FVector::ZeroVector;
	}
}

bool UCoverComponent::CalculateBestCrouchPosition(const TArray<AActor*>& Threats, TEnumAsByte<ETraceTypeQuery> TraceChannel, float TraceLength, ECrouchCoverPositions& OutPosition) const
{
    if (!CoverPoint || Threats.Num() == 0 || CoverFacingDirection.IsNearlyZero())
        return false;
    
    UWorld* World = GetWorld();
    if (!World)
        return false;

    const FVector CoverLocation = CoverPoint->GetActorLocation();
    const FVector CoverForward = CoverFacingDirection;
    const FVector CoverRight = FVector::CrossProduct(FVector::UpVector, CoverForward).GetSafeNormal();
	
    struct FCandidate
    {
        ECrouchCoverPositions Position;
        FVector Direction;
        float DotScore;
    };
    
    TArray<FCandidate> Candidates = {
        { ECrouchCoverPositions::Up,    CoverForward,   -2.0f },
        { ECrouchCoverPositions::Right, CoverRight,     -2.0f },
        { ECrouchCoverPositions::Left,  -CoverRight,    -2.0f }
    };
	
    for (FCandidate& Candidate : Candidates)
    {
        for (AActor* Threat : Threats)
        {
            if (!IsValid(Threat))
                continue;
            
            const FVector ToThreat = (Threat->GetActorLocation() - CoverLocation).GetSafeNormal2D();
            const float Dot = FVector::DotProduct(Candidate.Direction, ToThreat);
            
            if (Dot > Candidate.DotScore)
                Candidate.DotScore = Dot;
        }
    }
	
    Candidates.Sort([](const FCandidate& A, const FCandidate& B)
    {
        return A.DotScore > B.DotScore;
    });
	
    if (bDrawDebug)
    {
        DrawDebugLine(World, CoverLocation, CoverLocation + CoverForward * 200.0f,
            FColor::Blue, false, DebugDrawDuration, 0, 3.0f);
        DrawDebugLine(World, CoverLocation, CoverLocation + CoverRight * 200.0f,
            FColor::Red, false, DebugDrawDuration, 0, 3.0f);
        DrawDebugLine(World, CoverLocation, CoverLocation - CoverRight * 200.0f,
            FColor::Green, false, DebugDrawDuration, 0, 3.0f);
    }
	
    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(CoverDynamicPosition), false);
    TraceParams.AddIgnoredActor(CoverPoint);
    TraceParams.AddIgnoredActor(GetOwner());
    
    const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);
    
    for (const FCandidate& Candidate : Candidates)
    {
        if (Candidate.DotScore <= 0.0f)
            continue;
        
        const FVector TraceEnd = CoverLocation + (Candidate.Direction * TraceLength);
        
        FHitResult HitResult;
        const bool bBlocked = World->LineTraceSingleByChannel(
            HitResult, CoverLocation, TraceEnd, CollisionChannel, TraceParams);
    	
        if (bDrawDebug)
        {
            const FColor TraceColor = bBlocked ? FColor::Red : FColor::Green;
            DrawDebugLine(World, CoverLocation, bBlocked ? HitResult.Location : TraceEnd,
                TraceColor, false, DebugDrawDuration, 0, 1.5f);
        }
        
        if (!bBlocked)
        {
            OutPosition = Candidate.Position;
        	
            if (bDrawDebug)
            {
                DrawDebugLine(World, CoverLocation, TraceEnd,
                    FColor::Yellow, false, DebugDrawDuration, 0, 6.0f);
            }
            
            return true;
        }
    }
    
    return false;
}

bool UCoverComponent::CalculateBestStandPosition(const TArray<AActor*>& Threats, TEnumAsByte<ETraceTypeQuery> TraceChannel, float TraceLength, EStandCoverPositions& OutPosition) const
{
    if (!CoverPoint || Threats.Num() == 0 || CoverFacingDirection.IsNearlyZero())
        return false;
    
    UWorld* World = GetWorld();
    if (!World)
        return false;

    const FVector CoverLocation = CoverPoint->GetActorLocation();
    const FVector CoverForward = CoverFacingDirection;
    const FVector CoverRight = FVector::CrossProduct(FVector::UpVector, CoverForward).GetSafeNormal();
	
    struct FCandidate
    {
        EStandCoverPositions Position;
        FVector Direction;
        float DotScore;
    };
    
    TArray<FCandidate> Candidates = {
        { EStandCoverPositions::Right, CoverRight,    -2.0f },
        { EStandCoverPositions::Left,  -CoverRight,   -2.0f }
    };
    
    for (FCandidate& Candidate : Candidates)
    {
        for (AActor* Threat : Threats)
        {
            if (!IsValid(Threat))
                continue;
            
            const FVector ToThreat = (Threat->GetActorLocation() - CoverLocation).GetSafeNormal2D();
            const float Dot = FVector::DotProduct(Candidate.Direction, ToThreat);
            
            if (Dot > Candidate.DotScore)
                Candidate.DotScore = Dot;
        }
    }
    
    Candidates.Sort([](const FCandidate& A, const FCandidate& B)
    {
        return A.DotScore > B.DotScore;
    });
	
    if (bDrawDebug)
    {
        DrawDebugLine(World, CoverLocation, CoverLocation + CoverRight * 200.0f,
            FColor::Red, false, DebugDrawDuration, 0, 3.0f);
        DrawDebugLine(World, CoverLocation, CoverLocation - CoverRight * 200.0f,
            FColor::Green, false, DebugDrawDuration, 0, 3.0f);
    }
    
    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(CoverDynamicPosition), false);
    TraceParams.AddIgnoredActor(CoverPoint);
    TraceParams.AddIgnoredActor(GetOwner());
    
    const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);
    
    for (const FCandidate& Candidate : Candidates)
    {
        if (Candidate.DotScore <= 0.0f)
            continue;
        
        const FVector TraceEnd = CoverLocation + (Candidate.Direction * TraceLength);
        
        FHitResult HitResult;
        const bool bBlocked = World->LineTraceSingleByChannel(
            HitResult, CoverLocation, TraceEnd, CollisionChannel, TraceParams);
        
        if (bDrawDebug)
        {
            const FColor TraceColor = bBlocked ? FColor::Red : FColor::Green;
            DrawDebugLine(World, CoverLocation, bBlocked ? HitResult.Location : TraceEnd,
                TraceColor, false, DebugDrawDuration, 0, 1.5f);
        }
        
        if (!bBlocked)
        {
            OutPosition = Candidate.Position;
            
            if (bDrawDebug)
            {
                DrawDebugLine(World, CoverLocation, TraceEnd,
                    FColor::Yellow, false, DebugDrawDuration, 0, 6.0f);
            }
            
            return true;
        }
    }
    
    return false;
}

bool UCoverComponent::TestAndApplyBestCrouchPosition(const TArray<AActor*>& Threats, TEnumAsByte<ETraceTypeQuery> TraceChannel, float TraceLength)
{
	ECrouchCoverPositions NewPos;
	if (CalculateBestCrouchPosition(Threats, TraceChannel, TraceLength, NewPos))
	{
		SetCrouchCoverPosition(NewPos);
		UE_LOG(LogTemp, Warning, TEXT("Test: CrouchPosition set to %d"), static_cast<uint8>(NewPos));
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("Test: No valid crouch position found"));
	return false;
}

bool UCoverComponent::TestAndApplyBestStandPosition(const TArray<AActor*>& Threats, TEnumAsByte<ETraceTypeQuery> TraceChannel, float TraceLength)
{
	EStandCoverPositions NewPos;
	if (CalculateBestStandPosition(Threats, TraceChannel, TraceLength, NewPos))
	{
		SetStandCoverPosition(NewPos);
		UE_LOG(LogTemp, Warning, TEXT("Test: StandPosition set to %d"), static_cast<uint8>(NewPos));
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("Test: No valid stand position found"));
	return false;
}*/

bool UCoverComponent::IsInCover() const
{
	return bIsInCover;
}

bool UCoverComponent::IsOutOfCover() const
{
	if (!bIsInCover)
		return true;
	switch (CoverType)
	{
		case ECoverType::CrouchCover:
			return CrouchPosition != ECrouchCoverPositions::Idle;
		case ECoverType::StandCover:
			return StandPosition != EStandCoverPositions::Idle;
	}
	return false;
}

bool UCoverComponent::OutFromCover()
{
	if (CoverPoint && CoverPoint->CoverActor == GetOwner())
	{
		switch (CoverType)
		{
		case ECoverType::CrouchCover:
			if (CrouchPosition != ECrouchCoverPositions::Idle)
				return false;
			CrouchPosition = CoverPoint->CrouchPosition;
			OnPositionChanged.Broadcast(CoverType, static_cast<uint8>(CrouchPosition));
			break;
		case ECoverType::StandCover:
			if (StandPosition != EStandCoverPositions::Idle)
				return false;
			StandPosition = CoverPoint->StandPosition;
			OnPositionChanged.Broadcast(CoverType, static_cast<uint8>(StandPosition));
			break;
		}
		return true;
	}
	return false;
}

bool UCoverComponent::BackToCover()
{
	if (CoverPoint && CoverPoint->CoverActor == GetOwner())
	{
		switch (CoverType)
		{
		case ECoverType::CrouchCover:
			if (CrouchPosition == ECrouchCoverPositions::Idle)
				return false;
			CrouchPosition = ECrouchCoverPositions::Idle;
			OnPositionChanged.Broadcast(CoverType, static_cast<uint8>(CrouchPosition));
			break;
		case ECoverType::StandCover:
			if (StandPosition == EStandCoverPositions::Idle)
				return false;
			StandPosition = EStandCoverPositions::Idle;
			OnPositionChanged.Broadcast(CoverType, static_cast<uint8>(StandPosition));
			break;
		}
		return true;
	}
	return false;
}

void UCoverComponent::SetCrouchCoverPosition(ECrouchCoverPositions NewPosition)
{
	if (CoverPoint && CoverPoint->CoverActor == GetOwner() && CoverType == ECoverType::CrouchCover)
	{
		CrouchPosition = NewPosition;
		OnPositionChanged.Broadcast(CoverType, static_cast<uint8>(NewPosition));
	}
}

void UCoverComponent::SetStandCoverPosition(EStandCoverPositions NewPosition)
{
	if (CoverPoint && CoverPoint->CoverActor == GetOwner() && CoverType == ECoverType::StandCover)
	{
		StandPosition = NewPosition;
		OnPositionChanged.Broadcast(CoverType, static_cast<uint8>(NewPosition));
	}
}



