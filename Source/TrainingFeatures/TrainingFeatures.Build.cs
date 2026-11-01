using UnrealBuildTool;

public class TrainingFeatures : ModuleRules
{
    // Configures the runtime features and their direct engine dependencies.
    // Target is a read-only rules reference; no native pointers or ownership are involved.
    public TrainingFeatures(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicIncludePaths.Add(ModuleDirectory);
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "UMG"
        });
        PrivateDependencyModuleNames.AddRange(new string[] {
            "PhysicsCore", "Slate", "SlateCore"
        });
    }
}
