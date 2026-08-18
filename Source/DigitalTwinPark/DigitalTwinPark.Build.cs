// DigitalTwinPark - 智慧园区数字孪生
// 模块构建规则

using UnrealBuildTool;

public class DigitalTwinPark : ModuleRules
{
	public DigitalTwinPark(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",           // UI系统
			"HTTP",          // REST API数据请求
			"Niagara",       // 天气粒子特效
			"Slate",
			"SlateCore",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Json",
			"JsonUtilities",
		});

		// Pixel Streaming 支持
		if (Target.Type != TargetType.Server)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"PixelStreaming",
				"PixelStreamingInput",
			});
		}

		// 添加模块根目录和所有子目录到Include路径，支持相对include
		PublicIncludePaths.Add(ModuleDirectory);
		PublicIncludePaths.AddRange(new string[]
		{
			ModuleDirectory + "/Core",
			ModuleDirectory + "/Data",
			ModuleDirectory + "/Building",
			ModuleDirectory + "/UI",
			ModuleDirectory + "/Camera",
			ModuleDirectory + "/Environment",
			ModuleDirectory + "/Interaction",
		});
	}
}
