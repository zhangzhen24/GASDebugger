// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GASDebugger : ModuleRules
{
	public GASDebugger(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// Core modules
			"CoreUObject",
			"Engine",
			"InputCore",

			// UI modules
			"Slate",
			"SlateCore",

			// GAS modules (engine plugins)
			"GameplayAbilities",
			"GameplayTags",
		});

		// Editor-only modules
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd",
				"EditorStyle",
				"WorkspaceMenuStructure",
				"ToolMenus",
				"Projects",  // For IPluginManager (style icons)
				"PropertyEditor",  // For IDetailsView
			});
		}
	}
}
