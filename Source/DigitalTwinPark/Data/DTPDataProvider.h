// DigitalTwinPark - 智慧园区数字孪生
// 数据提供者接口 - 策略模式：统一Mock和HTTP数据源

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DTPDataTypes.h"
#include "DTPDataProvider.generated.h"

// 数据更新委托（C++内部使用，非蓝图）
DECLARE_MULTICAST_DELEGATE(FDTPOnDataUpdated);

UINTERFACE(MinimalAPI, Blueprintable)
class UDTPDataProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 数据提供者接口
 * MockDataProvider 和 HttpDataProvider 都实现此接口
 * 上层通过 UDTPDataSubsystem 统一调用，无需关心数据来源
 */
class DIGITALTWINPARK_API IDTPDataProvider
{
	GENERATED_BODY()

public:
	/** 开始拉取数据 */
	virtual void StartFetching() = 0;

	/** 停止拉取数据 */
	virtual void StopFetching() = 0;

	/** 设置数据更新间隔（秒） */
	virtual void SetUpdateInterval(float Seconds) = 0;

	/** 获取所有建筑数据 */
	virtual TArray<FDTPBuildingData> GetBuildingData() const = 0;

	/** 获取天气数据 */
	virtual FDTPWeatherData GetWeatherData() const = 0;

	/** 获取交通数据 */
	virtual FDTPTrafficData GetTrafficData() const = 0;

	/** 获取告警数据 */
	virtual TArray<FDTPAlertData> GetAlertData() const = 0;

	/** 数据更新回调 */
	FDTPOnDataUpdated OnDataUpdated;
};