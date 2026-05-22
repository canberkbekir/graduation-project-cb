// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class RizzGameEditor : ModuleRules
{
	public RizzGameEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"RizzGame",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"LevelEditor",
			"Slate",
			"SlateCore",
			"ToolMenus"
		});
	}
}
