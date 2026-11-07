using UnrealBuildTool;

public class TestFeatures : ModuleRules
{
    // Configures the runtime features and their direct engine dependencies.
    // Target is a read-only rules reference; no native pointers or ownership are involved.
    public TestFeatures(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicIncludePaths.Add(ModuleDirectory);
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "UMG"
        });
        PrivateDependencyModuleNames.AddRange(new string[] {
            "EngineSettings", "InputCore", "PhysicsCore", "Slate", "SlateCore"
        });
    }
}
