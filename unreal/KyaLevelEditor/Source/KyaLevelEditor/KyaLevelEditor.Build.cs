using UnrealBuildTool;

public class KyaLevelEditor : ModuleRules
{
    public KyaLevelEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "KyaLevel", "UnrealEd", "ProceduralMeshComponent" });
    }
}
