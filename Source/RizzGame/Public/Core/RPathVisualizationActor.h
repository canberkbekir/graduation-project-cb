#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RPathVisualizationActor.generated.h"

class USplineComponent;
class USplineMeshComponent;
class UStaticMeshComponent;
class URPathVisualizationConfig;

UCLASS()
class RIZZGAME_API APathVisualizationActor : public AActor
{
	GENERATED_BODY()

public:
	APathVisualizationActor();

	void BuildPath(const TArray<FVector>& WorldPoints, URPathVisualizationConfig* Config);

private:
	UPROPERTY()
	TObjectPtr<USplineComponent> PathSpline;

	UPROPERTY()
	TArray<TObjectPtr<USplineMeshComponent>> PathSegments;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> PathWaypoints;
};
