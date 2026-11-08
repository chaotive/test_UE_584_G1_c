using UnrealBuildTool;
using System.IO;
using System;
using System.Security.Cryptography;
using EpicGames.Core;

public class FeatureAngelScriptRuntime : ModuleRules
{
    // Links the pinned, prebuilt Win64 library without compiling or downloading the SDK.
    // Target is a borrowed managed rules reference; the external module owns no runtime objects.
    public FeatureAngelScriptRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;
        if (Target.Platform != UnrealTargetPlatform.Win64 || Target.Architecture != UnrealArch.X64)
        {
            throw new BuildException("AngelScript is supplied only for Win64 x64.");
        }
        if (Target.Configuration == UnrealTargetConfiguration.Debug && Target.bDebugBuildsActuallyUseDebugCRT)
        {
            throw new BuildException("The pinned AngelScript library uses /MD; a debug-CRT build needs its own library variant.");
        }

        string ManifestPath = Path.Combine(ModuleDirectory, "Dependency.json");
        JsonObject Manifest = JsonObject.Parse(File.ReadAllText(ManifestPath));
        Version DependencyVersion = Version.Parse(Manifest.GetStringField("Version"));
        string VersionDirectory = Path.Combine(ModuleDirectory, DependencyVersion.ToString());
        string Library = Path.Combine(VersionDirectory, "lib", "Win64", "Release", "angelscript.lib");
        string Header = Path.Combine(VersionDirectory, "include", "angelscript.h");
        string BuildInfoPath = Path.Combine(VersionDirectory, "BuildInfo.json");
        if (!File.Exists(Library) || !File.Exists(Header) || !File.Exists(BuildInfoPath))
        {
            throw new BuildException("AngelScript artifacts are missing. Run Source/ThirdParty/FeatureAngelScriptRuntime/Build-FeatureAngelScript.ps1.");
        }
        JsonObject BuildInfo = JsonObject.Parse(File.ReadAllText(BuildInfoPath));
        if (HashFile(ManifestPath) != BuildInfo.GetStringField("ManifestSha256")
            || HashFile(Library) != BuildInfo.GetStringField("LibrarySha256")
            || HashFile(Header) != BuildInfo.GetStringField("HeaderSha256"))
        {
            throw new BuildException("AngelScript artifacts do not match the pinned dependency. Run Build-FeatureAngelScript.ps1 -Rebuild.");
        }

        ExternalDependencies.Add(ManifestPath);
        ExternalDependencies.Add(BuildInfoPath);
        PublicSystemIncludePaths.Add(Path.GetDirectoryName(Header)!);
        PublicAdditionalLibraries.Add(Library);
        PublicDefinitions.Add("AS_USE_NAMESPACE");
        PublicDefinitions.Add($"FEATURE_ANGELSCRIPT_VERSION={DependencyVersion.Major * 10000 + DependencyVersion.Minor * 100 + DependencyVersion.Build}");
    }

    // Verifies the recorded dependency inputs and artifacts by their SHA-256 hashes.
    // Path and the returned hash are managed string references; the temporary byte array is garbage-collected.
    private static string HashFile(string Path)
    {
        return Convert.ToHexString(SHA256.HashData(File.ReadAllBytes(Path)));
    }
}
