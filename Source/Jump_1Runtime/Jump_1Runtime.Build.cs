// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Jump_1Runtime : ModuleRules
{
	public Jump_1Runtime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"AnimationCore",
				"AnimGraphRuntime",
				"AnimGraph",
				"BlueprintGraph",
				"Jump_1",
				"HTTP",
				"Json",
                "JsonUtilities"
            }
			);

		PrivateDependencyModuleNames.AddRange(new string[] { });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
        AddEngineThirdPartyPrivateStaticDependencies(Target, "libcurl");
        AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL");

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
