// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WalkthroughTest : ModuleRules
{
	public WalkthroughTest(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "HeadMountedDisplay", "OculusRift", "InputCore" });
	}
}
