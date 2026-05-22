

#pragma once

#include "CoreMinimal.h"
#include "CoverEnumsStructs.h"
#include "GameFramework/Actor.h"
#include "CoverPoint.generated.h"

UENUM(BlueprintType)
enum class ECoverState : uint8
{
	Available UMETA(DisplayName = "Available"),
	Occupied UMETA(DisplayName = "Occupied"),
	Reserved UMETA(DisplayName = "Reserved")
};

UCLASS()
class COVERSYSTEM_API ACoverPoint : public AActor
{
	GENERATED_BODY()

public:
	ACoverPoint();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover")
	TObjectPtr<UStaticMeshComponent> CoverMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover")
	ECoverState CoverState = ECoverState::Available;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	ECoverType CoverType = ECoverType::CrouchCover;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	bool bFindOutPositionDynamically = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	ECrouchCoverPositions CrouchPosition = ECrouchCoverPositions::Idle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	EStandCoverPositions StandPosition = EStandCoverPositions::Idle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover")
	TObjectPtr<AActor> CoverActor;
	
	UFUNCTION(BlueprintCallable, Category = "Cover")
	bool IsAvailable() const { return CoverState == ECoverState::Available && CoverActor == nullptr; }
	
	UFUNCTION(BlueprintCallable, Category = "Cover")
	bool IsReservedBy(AActor* Actor) const { return CoverState == ECoverState::Reserved && CoverActor == Actor; }

	UFUNCTION(BlueprintCallable, Category = "Cover")
	void OccupyCover(AActor* NewOccupant);
	
	UFUNCTION(BlueprintCallable, Category = "Cover")
	void ReserveCover(AActor* ReservingActor);

	UFUNCTION(BlueprintCallable, Category = "Cover")
	void ReleaseCover();

};
