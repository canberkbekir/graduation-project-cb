// RFogOfWarSettings.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RFogOfWarSettings.generated.h"

UCLASS(config=Game, defaultconfig, meta=(DisplayName="Fog of War"))
class RIZZGAME_API URFogOfWarSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	URFogOfWarSettings();

	static const URFogOfWarSettings* Get() { return GetDefault<URFogOfWarSettings>(); }

	UPROPERTY(config, EditAnywhere, Category="Grid",
	          meta=(ClampMin="10.0", ToolTip="Size of each fog grid cell in Unreal Units. Smaller = more detail, more memory. Tune via fow.stats."))
	float CellSize = 100.0f;

	UPROPERTY(config, EditAnywhere, Category="Grid",
	          meta=(ToolTip="If true, grid bounds are computed from the union of all ARFogRoom polygons at world begin play. Disable to set bounds manually."))
	bool bAutoComputeGridBounds = true;

	UPROPERTY(config, EditAnywhere, Category="Grid",
	          meta=(EditCondition="!bAutoComputeGridBounds", ToolTip="World-space XY minimum corner of the fog grid. Only used when bAutoComputeGridBounds is false."))
	FVector2D GridExtentMin = FVector2D(-5000.0f, -5000.0f);

	UPROPERTY(config, EditAnywhere, Category="Grid",
	          meta=(EditCondition="!bAutoComputeGridBounds", ToolTip="World-space XY maximum corner of the fog grid. Only used when bAutoComputeGridBounds is false."))
	FVector2D GridExtentMax = FVector2D(5000.0f, 5000.0f);

	UPROPERTY(config, EditAnywhere, Category="Debug",
	          meta=(ToolTip="Draw a colored box for each fog grid cell every LOS tick. Green = Visible, Yellow = Explored. Editor/Development only."))
	bool bDrawGrid = false;

	UPROPERTY(config, EditAnywhere, Category="Debug",
	          meta=(EditCondition="bDrawGrid", ToolTip="World Z height at which the debug grid boxes are drawn."))
	float DrawGridZ = 0.0f;

	UPROPERTY(config, EditAnywhere, Category="Line of Sight",
	          meta=(ClampMin="100.0", ToolTip="Default vision radius in Unreal Units for all party members."))
	float DefaultVisionRadius = 1200.0f;

	UPROPERTY(config, EditAnywhere, Category="Line of Sight",
	          meta=(ClampMin="0.016", ToolTip="Seconds between LOS recalculations. 0.1 = 10Hz. Lower = more responsive, higher CPU cost."))
	float LOSUpdateInterval = 0.1f;

	UPROPERTY(config, EditAnywhere, Category="Line of Sight",
	          meta=(ToolTip="Collision channel used for LOS raycasts. Walls and closed doors must block this channel."))
	TEnumAsByte<ECollisionChannel> LOSTraceChannel = ECC_Visibility;

	UPROPERTY(config, EditAnywhere, Category="Visual Fog",
	          meta=(ToolTip="Master switch for the BG3-style post-process fog overlay. Disable in main menus / character creation maps."))
	bool bEnableVisualFog = true;

	UPROPERTY(config, EditAnywhere, Category="Visual Fog",
	          meta=(AllowedClasses="/Script/Engine.MaterialInterface",
	                ToolTip="Post-process material asset (e.g. M_PPM_FogOfWar). Domain must be Post Process, Blendable Location: Before Tonemapping."))
	FSoftObjectPath FogPostProcessMaterial;

	UPROPERTY(config, EditAnywhere, Category="Visual Fog",
	          meta=(AllowedClasses="/Script/Engine.MaterialParameterCollection",
	                ToolTip="Material Parameter Collection for live-tunable visual fog parameters (e.g. MPC_FogOfWar)."))
	FSoftObjectPath FogParameterCollection;

	UPROPERTY(config, EditAnywhere, Category="Visual Fog",
	          meta=(ClampMin="0.0", ClampMax="20.0",
	                ToolTip="Reveal smoothing speed. 0 = instant pop-in, 6 ≈ 0.3s fade, 12 ≈ 0.15s. Higher = snappier."))
	float FogTemporalSpeed = 6.0f;
};
