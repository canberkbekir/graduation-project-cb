#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RPathVisualizationConfig.generated.h"

class UMaterialInterface;
class UStaticMesh;

/**
 * DataAsset controlling how the combat turn path preview is rendered.
 *
 * Setup:
 *   1. Right-click Content Browser → Miscellaneous → Data Asset → RPathVisualizationConfig.
 *   2. Assign PathSegmentMesh (any elongated UStaticMesh — tube, strip, arrow).
 *   3. Assign PathMaterial (your custom UMaterialInterface — scrolling arrows, glow, etc.).
 *   4. Open the Player Controller Blueprint → R|PathVisualization → assign this asset.
 */
UCLASS(BlueprintType)
class RIZZGAME_API URPathVisualizationConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Mesh tiled along each nav path segment. Deformed by USplineMeshComponent to follow the path curve. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "R|PathVisual|Mesh")
	TSoftObjectPtr<UStaticMesh> PathSegmentMesh;

	/** Material applied to every path segment mesh. Primary designer control — full material freedom. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "R|PathVisual|Mesh")
	TSoftObjectPtr<UMaterialInterface> PathMaterial;

	/**
	 * Which local axis of PathSegmentMesh points forward along the spline.
	 * 0 = X (default), 1 = Y, 2 = Z. Match this to how your mesh is authored.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "R|PathVisual|Mesh",
		meta = (ClampMin = "0", ClampMax = "2"))
	int32 ForwardAxis = 0;

	/**
	 * Scale of the mesh cross-section perpendicular to the path.
	 * X = width, Y = height. (1, 1) = original mesh size.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "R|PathVisual|Mesh")
	FVector2D MeshScale = FVector2D(1.f, 1.f);

	/**
	 * Offset of the mesh perpendicular to the path direction.
	 * X = side offset, Y = vertical offset (useful to raise the path above the ground).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "R|PathVisual|Mesh")
	FVector2D MeshOffset = FVector2D(0.f, 0.f);

	// --- Start point marker (always shown when set, independent of waypoints) ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "R|PathVisual|StartPoint")
	bool bShowStartPoint = false;

	/** Mesh placed at the first path point. Null = skip. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "R|PathVisual|StartPoint",
		meta = (EditCondition = "bShowStartPoint"))
	TSoftObjectPtr<UStaticMesh> StartPointMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "R|PathVisual|StartPoint",
		meta = (EditCondition = "bShowStartPoint"))
	TSoftObjectPtr<UMaterialInterface> StartPointMaterial;

	// --- End point marker ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "R|PathVisual|EndPoint")
	bool bShowEndPoint = false;

	/** Mesh placed at the last path point (destination). Null = skip. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "R|PathVisual|EndPoint",
		meta = (EditCondition = "bShowEndPoint"))
	TSoftObjectPtr<UStaticMesh> EndPointMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "R|PathVisual|EndPoint",
		meta = (EditCondition = "bShowEndPoint"))
	TSoftObjectPtr<UMaterialInterface> EndPointMaterial;

	// --- Middle waypoint markers ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "R|PathVisual|Waypoints")
	bool bShowWaypoints = false;

	/** Mesh placed at each middle path node. Null = skip. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "R|PathVisual|Waypoints",
		meta = (EditCondition = "bShowWaypoints"))
	TSoftObjectPtr<UStaticMesh> WaypointMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "R|PathVisual|Waypoints",
		meta = (EditCondition = "bShowWaypoints"))
	TSoftObjectPtr<UMaterialInterface> WaypointMaterial;
};
