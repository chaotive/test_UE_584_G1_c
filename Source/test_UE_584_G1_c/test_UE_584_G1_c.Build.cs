// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class test_UE_584_G1_c : ModuleRules
{
	public test_UE_584_G1_c(ReadOnlyTargetRules Target) : base(Target)
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
			"test_UE_584_G1_c",
			"test_UE_584_G1_c/Variant_Platforming",
			"test_UE_584_G1_c/Variant_Platforming/Animation",
			"test_UE_584_G1_c/Variant_Combat",
			"test_UE_584_G1_c/Variant_Combat/AI",
			"test_UE_584_G1_c/Variant_Combat/Animation",
			"test_UE_584_G1_c/Variant_Combat/Gameplay",
			"test_UE_584_G1_c/Variant_Combat/Interfaces",
			"test_UE_584_G1_c/Variant_Combat/UI",
			"test_UE_584_G1_c/Variant_SideScrolling",
			"test_UE_584_G1_c/Variant_SideScrolling/AI",
			"test_UE_584_G1_c/Variant_SideScrolling/Gameplay",
			"test_UE_584_G1_c/Variant_SideScrolling/Interfaces",
			"test_UE_584_G1_c/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
