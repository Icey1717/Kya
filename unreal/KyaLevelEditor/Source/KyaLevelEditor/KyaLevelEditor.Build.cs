using UnrealBuildTool;

public class KyaLevelEditor : ModuleRules
{
    public KyaLevelEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "KyaLevel", "UnrealEd",
            "ProceduralMeshComponent", "AssetRegistry", "AssetTools", "ToolMenus", "Slate", "SlateCore",
            "PropertyEditor", "MessageLog", "InputCore", "DesktopPlatform", "ApplicationCore",
            "GeometryFramework", "GeometryCore", "DynamicMesh", "ModelingToolsEditorMode", "ModelingComponents",
            "InteractiveToolsFramework", "EditorInteractiveToolsFramework" });
    }
}
