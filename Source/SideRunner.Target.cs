// Fill out your copyright notice in the Description page of Project Settings.
using UnrealBuildTool;
using System.Collections.Generic;

public class SideRunnerTarget : TargetRules
{
    public SideRunnerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        
        // Update from V2 to V5 for modern settings
        DefaultBuildSettings = BuildSettingsVersion.V5;
        
        // Use this for overriding settings while keeping compatibility
        bOverrideBuildEnvironment = true;
        
        // Update to C++20 standard
        CppStandard = CppStandardVersion.Cpp20;
        
        // Update include order
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        
        // Configure Windows platform settings
        WindowsPlatform.bStrictConformanceMode = true;
        
        // Keep your existing module names
        ExtraModuleNames.AddRange(new string[] { "SideRunner" });
    }
}