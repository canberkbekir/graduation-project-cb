// RFogOfWarSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "FogOfWar/RFogGrid.h"
#include "RFogOfWarSubsystem.generated.h"

class ARFogRoom;
class APostProcessVolume;
class UMaterialInstanceDynamic;
class UMaterialParameterCollection;
class UTexture2D;

UCLASS()
class RIZZGAME_API URFogOfWarSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	void RegisterRoom(ARFogRoom* Room);
	void UnregisterRoom(ARFogRoom* Room);

	UFUNCTION(BlueprintCallable, Category = "Rizz|FogOfWar")
	void RevealRoom(ARFogRoom* Room);

	UFUNCTION(BlueprintCallable, Category = "Rizz|FogOfWar")
	void HideRoom(ARFogRoom* Room);

	UFUNCTION(BlueprintCallable, Category = "Rizz|FogOfWar",
	          meta = (Categories = "FogRoom", ToolTip = "Reveal or hide a single room by its RoomId tag."))
	void SetRoomVisible(FGameplayTag RoomId, bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "Rizz|FogOfWar",
	          meta = (Categories = "FogRoom"))
	void RevealRoomById(FGameplayTag RoomId);

	UFUNCTION(BlueprintCallable, Category = "Rizz|FogOfWar")
	void RevealRoomGroup(FGameplayTag RoomGroup);

	UFUNCTION(BlueprintCallable, Category = "Rizz|FogOfWar",
			  meta = (Categories = "FogRoom"))
	void HideRoomById(FGameplayTag RoomId);

	UFUNCTION(BlueprintCallable, Category = "Rizz|FogOfWar")
	void HideRoomGroup(FGameplayTag RoomGroup);

	UFUNCTION(BlueprintCallable, Category = "Rizz|FogOfWar")
	void SetRoomGroupRevealed(FGameplayTag RoomGroup, bool bRevealed);

	UFUNCTION(BlueprintCallable, Category = "Rizz|FogOfWar")
	bool IsRoomGroupRevealed(FGameplayTag RoomGroup) const;

	UFUNCTION(BlueprintCallable, Category = "Rizz|FogOfWar")
	TArray<ARFogRoom*> GetAllRoomsInGroup(FGameplayTag RoomGroup) const;

	UFUNCTION(BlueprintCallable, Category = "Rizz|FogOfWar")
	ERFogState GetFogStateAtWorldPos(const FVector& WorldPos) const;

	UFUNCTION(BlueprintCallable, Category = "Rizz|FogOfWar")
	bool IsLocationVisible(const FVector& WorldPos) const;

	UFUNCTION(BlueprintCallable, Category = "Rizz|FogOfWar")
	bool IsLocationExplored(const FVector& WorldPos) const;

	const FRFogGrid& GetGrid() const { return Grid; }

	void SpawnLevelFogVolume();



	TMap<FGameplayTag, bool> GetRoomRevealStates() const;
	void RestoreGrid(const FRFogGrid& SavedGrid);
	void RestoreRoomStates(const TMap<FGameplayTag, bool>& SavedStates);

	UPROPERTY(EditAnywhere, Category = "Rizz|FogOfWar|Config")
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Rizz|FogOfWar|Config")
	float DefaultVisionRadius = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Rizz|FogOfWar|Config")
	float LOSUpdateInterval = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Rizz|FogOfWar|Config")
	TEnumAsByte<ECollisionChannel> LOSTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, Category = "Rizz|FogOfWar|Config",
	          meta = (ToolTip = "If true, grid bounds are computed from registered rooms at world begin play."))
	bool bAutoComputeGridBounds = true;

	UPROPERTY(EditAnywhere, Category = "Rizz|FogOfWar|Config",
	          meta = (EditCondition = "!bAutoComputeGridBounds"))
	FVector2D GridExtentMin = FVector2D(-5000.0f, -5000.0f);

	UPROPERTY(EditAnywhere, Category = "Rizz|FogOfWar|Config",
	          meta = (EditCondition = "!bAutoComputeGridBounds"))
	FVector2D GridExtentMax = FVector2D(5000.0f, 5000.0f);

private:
	void UpdateLOS();
	void InitializeGrid();
	void ComputeLOSForCharacter(const FVector& CharacterPos, float VisionRadius);
	bool TraceToCell(const FVector& From, const FVector2D& CellCenter) const;
	TArray<FVector> GatherVisionSources() const;
	void MarkRoomCellsExplored(ARFogRoom* Room);

	void DrawGridDebug() const;
	void InitializeVisualFog();
	void PublishGridToTexture(float DeltaSeconds);

	bool bDrawGrid = false;
	float DrawGridZ = 0.0f;

	FRFogGrid Grid;

	UPROPERTY()
	TArray<TObjectPtr<ARFogRoom>> RegisteredRooms;

	FTimerHandle LOSTimerHandle;

	UPROPERTY() TObjectPtr<UTexture2D>                   FogTexture  = nullptr;
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic>     FogPPMID    = nullptr;
	UPROPERTY() TObjectPtr<UMaterialParameterCollection> FogMPC      = nullptr;
	UPROPERTY() TObjectPtr<APostProcessVolume>           FogPPVolume = nullptr;

	bool bVisualFogEnabled = true;
	float FogTemporalSpeed = 6.0f;

	TArray<float> SmoothedCells;
	TArray<uint8> PixelScratch;
};
