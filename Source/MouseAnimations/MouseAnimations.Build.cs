// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MouseAnimations : ModuleRules
{
	public MouseAnimations(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
        "/media/kolya/sda/UnrealEngine5.0.3/Engine/Source/Editor/Sequencer/Public",
        "/media/kolya/sda/UnrealEngine5.0.3/Engine/Source/Editor/Sequencer/Private",
        "/media/kolya/sda/UnrealEngine5.0.3/Engine/Source/Editor/Sequencer/Private",
        "/media/kolya/sda/UnrealEngine5.0.3/Engine/Source/Editor/Sequencer/Private/DisplayNodes",
				"/media/kolya/sda/UnrealEngine5.0.3/Engine/Source/Editor/UnrealEd/Private" // TODO: Fix this, for now it's needed for the fbx exporter
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
        "/media/kolya/sda/UnrealEngine5.0.3/Engine/Source/Editor/Sequencer/Public",
        "/media/kolya/sda/UnrealEngine5.0.3/Engine/Source/Editor/Sequencer/Private",
        "/media/kolya/sda/UnrealEngine5.0.3/Engine/Source/Editor/Sequencer/Private",
        "/media/kolya/sda/UnrealEngine5.0.3/Engine/Source/Editor/Sequencer/Private/DisplayNodes",
				"/media/kolya/sda/UnrealEngine5.0.3/Engine/Source/Editor/UnrealEd/Private" // TODO: Fix this, for now it's needed for the fbx exporter
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
				"LevelSequence",
				"LevelSequenceEditor",
				"AssetRegistry",
				"MovieScene",
        "MovieSceneTools",
        "ControlRig",
        "RigVM"
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
