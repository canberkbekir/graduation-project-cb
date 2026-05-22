#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Events/WorldViewEvents.h"
#include "RWorldViewOutlineConfig.generated.h"

class UMaterialInterface;

/**
 * One entry in the outline tag map.
 * Ties an actor tag to a Custom Depth stencil value and which world view should show it.
 * Stencil values 1-3 are reserved by URSelectionComponent (Hover/Selected/ActiveTurn).
 * Use 4+ here.
 */
USTRUCT(BlueprintType)
struct FOutlineTagEntry
{
	GENERATED_BODY()

	/** Custom Depth stencil value written to the actor's primitives. Must be unique per color in your outline material. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=4, ClampMax=255))
	int32 StencilValue = 4;

	/** Which world view activates this outline. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EWorldView WorldView = EWorldView::Physical;
};

/**
 * DataAsset that configures the world-view outline system.
 *
 * Setup:
 *   1. Create a DA_WorldViewOutlines asset (right-click Content Browser → Miscellaneous → DataAsset).
 *   2. Fill TagConfig with actor tags, their stencil values, and which world view they belong to.
 *   3. Assign your outline post-process material to OutlineMaterial.
 *   4. Point to this asset in Project Settings → Game → World View Outlines.
 *   5. Add one PostProcessVolume to your level, tag it with OutlineVolumeTag ("OutlineVolume" by default),
 *      set Infinite Extent, Priority -1, leave it Enabled.
 */
UCLASS(BlueprintType)
class RIZZGAME_API URWorldViewOutlineConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Maps actor tags to stencil values + which world view shows them.
	 * Each tag must appear at most once — this prevents stencil conflicts between views.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tags")
	TMap<FName, FOutlineTagEntry> TagConfig;

	/**
	 * Tag on the PostProcessVolume in the level that hosts the outline material.
	 * This volume should be always-enabled with Infinite Extent and low Priority.
	 * Separate from the NetworkWorld/PhysicalWorld volumes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Volume")
	FName OutlineVolumeTag = FName("OutlineVolume");

	/**
	 * The outline post-process material. Injected once into the OutlineVolume on startup.
	 * Should read CustomStencil and draw a different outline color per stencil value.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Material")
	TSoftObjectPtr<UMaterialInterface> OutlineMaterial;

	/**
	 * Returns the stencil value if the actor has a tag that belongs to the given world view.
	 * Returns 0 if no matching tag is found.
	 */
	int32 GetStencilForActor(const AActor* Actor, EWorldView View) const;

	/** Returns true if the actor has any tag present in TagConfig (regardless of world view). */
	bool ActorHasAnyOutlineTag(const AActor* Actor) const;
};
