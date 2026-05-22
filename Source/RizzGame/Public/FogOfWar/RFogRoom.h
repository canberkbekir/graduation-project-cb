// RFogRoom.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "FogOfWar/RFogGrid.h"
#include "RFogRoom.generated.h"

class USplineComponent;
class UBillboardComponent;
class URFogOfWarSubsystem;

UCLASS()
class RIZZGAME_API ARFogRoom : public AActor
{
	GENERATED_BODY()

public:
	ARFogRoom();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	FGameplayTag GetRoomId() const { return RoomId; }
	FGameplayTag GetRoomGroup() const { return RoomGroup; }
	bool IsRevealed() const { return bRevealed; }
	const TArray<FVector2D>& GetPolygonPoints() const { return CachedPolygon; }

	void SetRevealed(bool bInRevealed);

	bool ContainsPoint2D(const FVector2D& Point) const;
	void GetOverlappingGridCells(const FRFogGrid& Grid, TArray<FIntPoint>& OutCells) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rizz|FogRoom")
	TArray<TObjectPtr<AActor>> ContainedActors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|FogRoom",
	          meta = (ClampMin = "50.0", ToolTip = "Vertical range centered on this actor's Z used when scanning for contained actors."))
	float RoomHeight = 400.0f;

	UFUNCTION(CallInEditor, Category = "Rizz|FogRoom")
	void RefreshContainedActors();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rizz|Components")
	TObjectPtr<USplineComponent> RoomSpline;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rizz|Components")
	TObjectPtr<UBillboardComponent> Billboard;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|FogRoom",
	          meta = (Categories = "FogRoom", ToolTip = "Unique tag for this room. e.g. FogRoom.Dungeon.Cell1. Used by SetRoomVisible on the subsystem."))
	FGameplayTag RoomId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|FogRoom",
	          meta = (Categories = "FogRoom", ToolTip = "Group tag shared by multiple rooms. e.g. FogRoom.Dungeon. Use RevealRoomGroup/HideRoomGroup to toggle all at once."))
	FGameplayTag RoomGroup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|FogRoom")
	bool bStartRevealed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|Debug")
	bool bShowDebug = true;

private:
	bool bRevealed = false;

	UPROPERTY()
	TObjectPtr<URFogOfWarSubsystem> CachedSubsystem;

	TArray<FVector2D> CachedPolygon;

	void RebuildPolygon();
	void AutoDetectContainedActors();
	void DrawDebugRoom() const;
	FColor GetGroupColor() const;
};
