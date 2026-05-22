using UnrealBuildTool;

public class ApexRacer : ModuleRules
{
	public ApexRacer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"ChaosVehicles",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateIncludePaths.Add(ModuleDirectory);
	}
}
