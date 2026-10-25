// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SimulateThis : ModuleRules
{
	public SimulateThis(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			[
				"Core", "CoreUObject", "Engine", "GameplayTags"
			]
		);
	}
}