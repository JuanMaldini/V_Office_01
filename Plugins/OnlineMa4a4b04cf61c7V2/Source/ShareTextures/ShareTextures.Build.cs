// Copyright ShareTextures 2024. All Rights Reserved.

using UnrealBuildTool;

public class ShareTextures : ModuleRules
{
	public ShareTextures(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"BlueprintMaterialTextureNodes", 
				"Blutility", 
				"Core",
				"CoreUObject",
				"DeveloperSettings", 
				"EditorSubsystem", 
				"Engine",
				"FileUtilities",
				"HTTP", 
				"Json", 
				"JsonUtilities",
				"RenderCore",
				"Projects",
				"Slate",
				"SlateCore", 
				"ToolMenus",
				"UMG",
				"UMGEditor",
				"UnrealEd",
			}
		);
	}
}