// RCameraWall.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "RCameraWall.generated.h"

class URCameraWallSubsystem;
class UBillboardComponent;
class USplineComponent;

/** A single 2D wall segment (between two consecutive spline points). */
USTRUCT()
struct FWallSegment2D
{
	GENERATED_BODY()

	FVector2D A = FVector2D::ZeroVector;
	FVector2D B = FVector2D::ZeroVector;
	FVector2D Normal = FVector2D::ZeroVector;
	FVector2D AB = FVector2D::ZeroVector;
	float ABLenSq = 0.0f;
};

/**
 * A polyline wall placed in the level that restricts camera pawn movement.
 * Defined by a spline — each pair of consecutive points is a wall segment.
 * Level designers can add/remove points by right-clicking the spline in the viewport.
 */
UCLASS()
class RIZZGAME_API ARCameraWall : public AActor
{
	GENERATED_BODY()

public:
	ARCameraWall();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	/** Enable or disable this wall at runtime. */
	UFUNCTION(BlueprintCallable, Category = "Rizz|CameraWall")
	void SetWallEnabled(bool bInEnabled);

	/** Returns all 2D segments derived from the spline points. */
	const TArray<FWallSegment2D>& GetSegments() const { return CachedSegments; }

	/** Returns the number of wall segments. */
	int32 GetSegmentCount() const { return CachedSegments.Num(); }

	FName GetWallId() const { return WallId; }
	FGameplayTag GetWallGroup() const { return WallGroup; }
	bool IsWallEnabled() const { return bEnabled; }
	bool IsTwoSided() const { return bTwoSided; }
	bool BlocksCameraActor() const { return bBlockCameraActor; }

protected:
	/* ---------- Components ---------- */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rizz|Components")
	TObjectPtr<USplineComponent> WallSpline;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rizz|Components")
	TObjectPtr<UBillboardComponent> Billboard;
#endif

	/* ---------- Wall Settings ---------- */

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|CameraWall", meta = (ToolTip = "Unique name for this wall. Use this ID to enable/disable a specific wall from Blueprints or C++ via the CameraWall Subsystem."))
	FName WallId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|CameraWall", meta = (Categories = "CameraWall", ToolTip = "Group tag shared by multiple walls. Toggle all walls in the same group at once using SetWallGroupEnabled on the CameraWall Subsystem. Example: CameraWall.Combat, CameraWall.Zone1"))
	FGameplayTag WallGroup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|CameraWall", meta = (ToolTip = "If true, this wall blocks the camera from the start of play. If false, it starts disabled and must be enabled at runtime via Blueprint or C++."))
	bool bStartEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|CameraWall", meta = (ToolTip = "If true, blocks camera movement from BOTH sides of the wall. If false (default), the camera can only be blocked from the arrow/normal side and can pass through from the other side."))
	bool bTwoSided = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|CameraWall", meta = (ToolTip = "Reverses the blocking direction (the arrow side). Use this if the wall is blocking the wrong side. The debug arrow always points toward the blocked side."))
	bool bFlipNormal = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|CameraWall", meta = (ToolTip = "If true, the wall also blocks the camera actor itself (end of the spring arm), not just the camera pivot. Prevents the view from extending past the wall when zoomed out."))
	bool bBlockCameraActor = true;

	/* ---------- Debug ---------- */

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|Debug", meta = (ToolTip = "Show the wall outline and normal arrows in the editor viewport and during play. Disable to hide debug visuals."))
	bool bShowDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|Debug", meta = (ClampMin = "10.0", ToolTip = "Height of the debug wall visualization in Unreal units. Only affects the visual — the actual blocking is 2D (XY plane)."))
	float WallHeight = 1000.0f;

private:
	bool bEnabled = true;

	UPROPERTY()
	TObjectPtr<URCameraWallSubsystem> CachedSubsystem;

	/** Pre-computed 2D segments from the spline, rebuilt when spline changes. */
	TArray<FWallSegment2D> CachedSegments;

	/** Rebuild CachedSegments from the current spline points. */
	void RebuildSegments();

	/** Draw debug visualization for all segments. */
	void DrawDebugWall() const;

	/** Generates a deterministic color from the WallGroup tag hash. */
	FColor GetGroupColor() const;
};
