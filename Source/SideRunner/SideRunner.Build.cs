// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class SideRunner : ModuleRules
{
	public SideRunner(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// PERFORMANCE: Core dependencies for optimized builds
        PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore" 
		});

		// PERFORMANCE: Private dependencies for specific features
        PrivateDependencyModuleNames.AddRange(new string[] { 
			"AudioMixer"			// For optimized audio
		});

		// PERFORMANCE: Enable optimizations for shipping builds
		if (Target.Configuration == UnrealTargetConfiguration.Shipping)
		{
			bUseUnity = true;
			MinFilesUsingPrecompiledHeaderOverride = 1;
		}

		// PERFORMANCE: Enable faster compilation in development
		if (Target.Configuration == UnrealTargetConfiguration.Development)
		{
			bUseUnity = true;
		}

		// PERFORMANCE: Compiler optimizations
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
		
		// PERFORMANCE: Use precompiled headers for faster builds
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	}
}
