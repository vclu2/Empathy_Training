// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Jump_1 : ModuleRules
{
    public Jump_1(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "GraphEditor",
            "PropertyEditor",
            "EditorStyle",
            "AnimGraphRuntime",
            "AnimGraph",
            "BlueprintGraph",
            "HTTP",
            "Json",
            "JsonUtilities"
        }
        );

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
        AddEngineThirdPartyPrivateStaticDependencies(Target, "libcurl");
        AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL");

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
