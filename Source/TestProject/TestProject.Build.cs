// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TestProject : ModuleRules
{
	public TestProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] 
			{ 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore", 
				"EnhancedInput", 
				"Json", 
				"JsonUtilities", 
				"FunctionalTesting", 
				"AutomationDriver"
            });

		if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[] {
					"UnrealEd",
					"UATHelper",
					"SettingsEditor"
                }
            );
        }

        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
						"CoreUObject",
						"Engine",
						"Slate",
						"SlateCore",
						"AutomationDriver",

						"Core",
						"ApplicationCore",
						"Json"

				// ... add private dependencies that you statically link with here ...	
			}
			);

        PublicIncludePaths.Add("TestProject");
	}
}
