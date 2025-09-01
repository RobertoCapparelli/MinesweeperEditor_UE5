using UnrealBuildTool;

public class MinesweeperEditor : ModuleRules
{
    public MinesweeperEditor(ReadOnlyTargetRules Target) : base(Target)
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
			}
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "InputCore",
                "ToolMenus",
                "CoreUObject",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "LevelEditor",
			}
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
			}
            );
    }
}
