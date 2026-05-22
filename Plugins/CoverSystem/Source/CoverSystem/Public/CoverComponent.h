

#pragma once

#include "CoreMinimal.h"
#include "CoverEnumsStructs.h"
#include "Components/ActorComponent.h"
#include "CoverPoint.h"
#include "CoverComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoverPointEvent, ACoverPoint*, CoverPoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCoverPositionChanged, ECoverType, CoverType, uint8, NewPosition);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), DisplayName = "Cover Component")
class COVERSYSTEM_API UCoverComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UCoverComponent();
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// Cover lifecycle
	UFUNCTION(BlueprintCallable, Category = "Cover")
	void OccupyCoverPoint(ACoverPoint* NewCoverPoint, bool bIsPhysicllyReached = true);
	UFUNCTION(BlueprintCallable, Category = "Cover")
	void OccupyCoverPointWithoutReservation(ACoverPoint* NewCoverPoint, bool bIsPhysicllyReached = true);
	UFUNCTION(BlueprintCallable, Category = "Cover")
	void ReserveCoverPoint(ACoverPoint* NewCoverPoint);
	UFUNCTION(BlueprintCallable, Category = "Cover")
	void ClearCoverPoint();
    
	// Physical state
	UFUNCTION(BlueprintCallable, Category = "Cover")
	void SetIsInCover(bool bNewInCover);
	UFUNCTION(BlueprintPure, Category = "Cover")
	bool IsInCover() const;
	UFUNCTION(BlueprintPure, Category = "Cover")
	bool IsOutOfCover() const;
    
	// Position transitions
	UFUNCTION(BlueprintCallable, Category = "Cover")
	bool OutFromCover();
	UFUNCTION(BlueprintCallable, Category = "Cover")
	bool BackToCover();
	UFUNCTION(BlueprintCallable, Category = "Cover")
	void SetCrouchCoverPosition(ECrouchCoverPositions NewPosition);
	UFUNCTION(BlueprintCallable, Category = "Cover")
	void SetStandCoverPosition(EStandCoverPositions NewPosition);
    
	// Getters
	UFUNCTION(BlueprintPure, Category = "Cover")
	ACoverPoint* GetCoverPoint() const { return CoverPoint; }
	UFUNCTION(BlueprintPure, Category = "Cover")
	ECoverType GetCoverType() const { return CoverType; }
	UFUNCTION(BlueprintPure, Category = "Cover")
	ECrouchCoverPositions GetCrouchPosition() const { return CrouchPosition; }
	UFUNCTION(BlueprintPure, Category = "Cover")
	EStandCoverPositions GetStandPosition() const { return StandPosition; }
	
	UPROPERTY(BlueprintAssignable, Category = "Cover|Events")
	FOnCoverPointEvent OnCoverReserved;
	UPROPERTY(BlueprintAssignable, Category = "Cover|Events")
	FOnCoverPointEvent OnCoverOccupied;
	UPROPERTY(BlueprintAssignable, Category = "Cover|Events")
	FOnCoverPointEvent OnCoverFreed;
	UPROPERTY(BlueprintAssignable, Category = "Cover|Events")
	FOnCoverPointEvent OnEnteredCover;
	UPROPERTY(BlueprintAssignable, Category = "Cover|Events")
	FOnCoverPointEvent OnExitedCover;
	UPROPERTY(BlueprintAssignable, Category = "Cover|Events")
	FOnCoverPositionChanged OnPositionChanged;
	
	
	/*UFUNCTION(BlueprintCallable, Category = "Cover|Dynamic Position")
	bool CalculateBestCrouchPosition(const TArray<AActor*>& Threats, TEnumAsByte<ETraceTypeQuery> TraceChannel, float TraceLength, ECrouchCoverPositions& OutPosition) const;

	UFUNCTION(BlueprintCallable, Category = "Cover|Dynamic Position")
	bool CalculateBestStandPosition(const TArray<AActor*>& Threats, TEnumAsByte<ETraceTypeQuery> TraceChannel, float TraceLength, EStandCoverPositions& OutPosition) const;
	
	UFUNCTION(BlueprintCallable, Category = "Cover|Debug Test")
	bool TestAndApplyBestCrouchPosition(const TArray<AActor*>& Threats, TEnumAsByte<ETraceTypeQuery> TraceChannel, float TraceLength = 500.0f);

	UFUNCTION(BlueprintCallable, Category = "Cover|Debug Test")
	bool TestAndApplyBestStandPosition(const TArray<AActor*>& Threats, TEnumAsByte<ETraceTypeQuery> TraceChannel, float TraceLength = 500.0f);
	
	UFUNCTION(BlueprintCallable, Category = "Cover|Debug")
	void SetDebugDrawEnabled(bool bEnabled) { bDrawDebug = bEnabled; }
	
	UFUNCTION(BlueprintPure, Category = "Cover")
	FVector GetCoverFacingDirection() const { return CoverFacingDirection; }*/
protected:
	/*UPROPERTY(VisibleAnywhere, Category = "Cover|Dynamic Position")
	FVector CoverFacingDirection = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, Category = "Cover|Debug")
	bool bDrawDebug = false;

	UPROPERTY(EditAnywhere, Category = "Cover|Debug", meta = (EditCondition = "bDrawDebug"))
	float DebugDrawDuration = 5.0f;*/
	
	ECoverType CoverType = ECoverType::CrouchCover;
	ECrouchCoverPositions CrouchPosition = ECrouchCoverPositions::Idle;
	EStandCoverPositions StandPosition = EStandCoverPositions::Idle;
	
	UPROPERTY()
	TObjectPtr<ACoverPoint> CoverPoint;
	bool bFindOutPositionDynamically = false;
	bool bIsInCover = false;
};
