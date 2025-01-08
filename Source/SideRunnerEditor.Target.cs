using UnrealBuildTool;
using System.Collections.Generic;

public class SideRunnerEditorTarget : TargetRules
{
    public SideRunnerEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        CppStandard = CppStandardVersion.Cpp20;

        // Instead of BuildEnvironment = Unique, use this:
        bOverrideBuildEnvironment = true;

        ExtraModuleNames.AddRange(new string[] { "SideRunner" });
    }
}