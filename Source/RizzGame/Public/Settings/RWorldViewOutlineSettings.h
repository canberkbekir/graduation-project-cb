#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Settings/RWorldViewOutlineConfig.h"
#include "RWorldViewOutlineSettings.generated.h"

/**
 * Project Settings → Game → World View Outlines
 *
 * Points to the DataAsset that holds all outline tag/stencil/material configuration.
 * Create a DA_WorldViewOutlines asset in the Content Browser and assign it here.
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="World View Outlines"))
class RIZZGAME_API URWorldViewOutlineSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	URWorldViewOutlineSettings();

	/** The DataAsset that configures tags, stencil values, and the outline material. */
	UPROPERTY(config, EditAnywhere, Category="Config")
	TSoftObjectPtr<URWorldViewOutlineConfig> OutlineConfig;

	/** Loads and returns the config asset synchronously. Returns nullptr if not set or failed to load. */
	static URWorldViewOutlineConfig* LoadConfig();
};
