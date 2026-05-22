// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * Adds a "Network World" toggle button to the level editor toolbar.
 * Lets the level designer preview the Network World post process
 * in the viewport without entering PIE.
 *
 * Setup: create MPC_WorldState at /Game/Materials/MPC_WorldState
 * with a scalar parameter named "NetworkWorldBlend".
 */
class FRWorldViewEditorExtension
{
public:
	// Registers the toolbar button
	static void Register();

private:
	// Adds the button via the toolbar extender
	static void RegisterMenus();
	static void AddToolbarButton(FToolBarBuilder& Builder);

	// Button actions
	static void ToggleNetworkWorld();
	static bool IsNetworkWorldActive();

	// Enables/disables post-process volume
	static void SetPostProcessEnabled(bool bEnabled);

	// Tracks state
	static bool bNetworkWorldActive;
};