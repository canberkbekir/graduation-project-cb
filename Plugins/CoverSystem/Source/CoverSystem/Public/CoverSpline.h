
#pragma once

#include "CoreMinimal.h"
#include "CoverPoint.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "CoverSpline.generated.h"

UCLASS()
class COVERSYSTEM_API ACoverSpline : public AActor
{
	GENERATED_BODY()
	
public:
	ACoverSpline();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover")
	TObjectPtr<USplineComponent> SplineComponent;
	
	UFUNCTION(CallInEditor, Category = "Cover")
	void GenerateCoverPoints();

	UFUNCTION(CallInEditor, Category = "Cover")
	void ClearCoverPoints();

	UPROPERTY(EditAnywhere, Category = "Cover")
	float SphereRadius = 32.0f;

	UPROPERTY(VisibleAnywhere, Category = "Cover")
	TSubclassOf<ACoverPoint> CoverPointClass = ACoverPoint::StaticClass();
	
	UPROPERTY(EditAnywhere, Category = "Cover")
	TObjectPtr<UStaticMesh> CoverPointMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	ECoverType CoverType = ECoverType::CrouchCover;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	bool bFindOutPositionDynamically = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	ECrouchCoverPositions CrouchPosition = ECrouchCoverPositions::Idle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	EStandCoverPositions StandPosition = EStandCoverPositions::Idle;

protected:
	UPROPERTY()
	TArray<ACoverPoint*> SpawnedCoverPoints;
};
