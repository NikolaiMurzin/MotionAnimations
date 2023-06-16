// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

using UnrealBuildTool;

public class MotionAnimations : ModuleRules
{
    public MotionAnimations(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
            }
            );


        PrivateIncludePaths.AddRange(
            new string[] {
            }
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Sequencer",
				// ... add other public dependencies that you statically link with here ...
			}
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Projects",
                "InputCore",
                "EditorFramework",
                "UnrealEd",
                "Sequencer",
                "ToolMenus",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Niagara",
                "LevelSequence",
                "LevelSequenceEditor",
                "AssetRegistry",
                "MovieScene",
                "MovieSceneTools",
                "MovieSceneTracks",
                "ControlRig",
                "RigVM",
                "Json",
                "JsonUtilities"
				// ... add private dependencies that you statically link with here ...	
			}
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );
    }
}
