// DigitalTwinPark - 智慧园区数字孪生
// 模拟数据提供者 - Demo阶段生成随机数据

#pragma once

#include "CoreMinimal.h"
#include "DTPDataProvider.h"
#include "DTPMockDataProvider.generated.h"

/**
 * 模拟数据提供者
 * 在Demo阶段生成随机但合理的IoT数据，用于UI开发和演示
 */
UCLASS(BlueprintType, Blueprintable)
class DIGITALTWINPARK_API UDTPMockDataProvider : public UObject, public IDTPDataProvider
{
	GENERATED_BODY()

public:
	UDTPMockDataProvider();

	// ~IDTPDataProvider 接口实现
	virtual void StartFetching() override;
	virtual void StopFetching() override;
	virtual void SetUpdateInterval(float Seconds) override;
	virtual TArray<FDTPBuildingData> GetBuildingData() const override;
	virtual FDTPWeatherData GetWeatherData() const override;
	virtual FDTPTrafficData GetTrafficData() const override;
	virtual TArray<FDTPAlertData> GetAlertData() const override;

private:
	/** 定时更新数据 */
	void TickUpdate();

	/** 生成模拟建筑数据 */
	void GenerateMockBuildingData();

	/** 生成模拟天气数据 */
	void GenerateMockWeatherData();

	/** 生成模拟交通数据 */
	void GenerateMockTrafficData();

	/** 生成模拟告警 */
	void GenerateMockAlerts();

	/** 生成单个设备数据 */
	static FDTPDeviceData GenerateDeviceData(const FString& BuildingId, const FString& DeviceSuffix,
		EDTPDeviceType Type, float MinVal, float MaxVal, const FString& Unit);

	// 缓存数据
	TArray<FDTPBuildingData> CachedBuildingData;
	FDTPWeatherData CachedWeatherData;
	FDTPTrafficData CachedTrafficData;
	TArray<FDTPAlertData> CachedAlertData;

	// 定时器
	FTimerHandle UpdateTimerHandle;
	float UpdateInterval = 2.0f;
	bool bIsFetching = false;

	// 随机种子
	FRandomStream RandomStream;
};