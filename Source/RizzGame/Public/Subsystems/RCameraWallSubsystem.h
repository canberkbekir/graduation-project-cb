// RCameraWallSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "Events/CameraWallEvents.h"
#include "RCameraWallSubsystem.generated.h"

class ARCameraWall;

/**
 * Tracks all camera walls in the world and provides boundary clamping.
 */
UCLASS()
class RIZZGAME_API URCameraWallSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void RegisterWall(ARCameraWall* Wall);
	void UnregisterWall(ARCameraWall* Wall);

	/** Enable or disable a single camera wall by its unique WallId. The WallId is set on the RCameraWall actor in the level. */
	UFUNCTION(BlueprintCallable, Category = "Rizz|CameraWall", meta = (ToolTip = "Enable or disable a specific camera wall by its WallId. Set the WallId on the RCameraWall actor placed in the level."))
	void SetWallEnabled(FName WallId, bool bEnabled);

	/** Enable or disable ALL camera walls that share the given group tag. */
	UFUNCTION(BlueprintCallable, Category = "Rizz|CameraWall", meta = (ToolTip = "Enable or disable ALL camera walls with the given WallGroup tag at once. Useful for toggling an entire zone (e.g. CameraWall.Combat)."))
	void SetWallGroupEnabled(FGameplayTag WallGroup, bool bEnabled);

	/** Returns all camera wall actors that have the given group tag. */
	UFUNCTION(BlueprintCallable, Category = "Rizz|CameraWall", meta = (ToolTip = "Returns an array of all RCameraWall actors in the level that share the given WallGroup tag."))
	TArray<ARCameraWall*> GetAllWallsInGroup(FGameplayTag WallGroup) const;

	/** Returns true if at least one wall in the group is currently enabled. */
	UFUNCTION(BlueprintCallable, Category = "Rizz|CameraWall", meta = (ToolTip = "Returns true if any camera wall with the given WallGroup tag is currently enabled/active."))
	bool IsWallGroupEnabled(FGameplayTag WallGroup) const;

	/**
	 * Clamps NewPos so it doesn't cross any active wall from OldPos.
	 * Multi-iteration to handle corner cases.
	 */
	FVector ClampPositionToWalls(const FVector& OldPos, const FVector& NewPos) const;

	/**
	 * Clamps the camera actor position (end of spring arm) against walls
	 * that have bBlockCameraActor enabled. Returns the clamped camera position.
	 */
	FVector ClampCameraActorToWalls(const FVector& OldCameraPos, const FVector& NewCameraPos) const;

private:
	FVector ClampPositionInternal(const FVector& OldPos, const FVector& NewPos, bool bCameraActorOnly) const;

	void OnCameraWallToggled(const FCameraWallToggled& Event);
	void OnCameraWallGroupToggled(const FCameraWallGroupToggled& Event);

	UPROPERTY()
	TArray<TObjectPtr<ARCameraWall>> RegisteredWalls;

	FDelegateHandle WallToggledHandle;
	FDelegateHandle WallGroupToggledHandle;
};
