// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RizzGame : ModuleRules
{
	public RizzGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"UMG",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"Slate",
			"SlateCore",
			"DeveloperSettings",
			"HAIPro",
			"CoverSystem"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new[]
		{
			"RizzGame",
			"RizzGame/Public",
			"RizzGame/Variants/Strategy",
			"RizzGame/Variants/TwinStick",
			"RizzGame/Variants/TwinStick/AI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}