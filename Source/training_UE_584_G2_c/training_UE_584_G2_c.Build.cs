// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class training_UE_584_G2_c : ModuleRules
{
	public training_UE_584_G2_c(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"TrainingFeatures",
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "SlateCore" });

		PublicIncludePaths.AddRange(new string[] {
			"training_UE_584_G2_c",
			"training_UE_584_G2_c/Variant_Platforming",
			"training_UE_584_G2_c/Variant_Platforming/Animation",
			"training_UE_584_G2_c/Variant_Combat",
			"training_UE_584_G2_c/Variant_Combat/AI",
			"training_UE_584_G2_c/Variant_Combat/Animation",
			"training_UE_584_G2_c/Variant_Combat/Gameplay",
			"training_UE_584_G2_c/Variant_Combat/Interfaces",
			"training_UE_584_G2_c/Variant_Combat/UI",
			"training_UE_584_G2_c/Variant_SideScrolling",
			"training_UE_584_G2_c/Variant_SideScrolling/AI",
			"training_UE_584_G2_c/Variant_SideScrolling/Gameplay",
			"training_UE_584_G2_c/Variant_SideScrolling/Interfaces",
			"training_UE_584_G2_c/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
