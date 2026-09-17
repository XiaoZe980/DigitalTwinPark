// DigitalTwinPark - 智慧园区数字孪生
// 模拟数据提供者实现

#include "DTPMockDataProvider.h"
#include "DigitalTwinPark.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"

// ============================================================================
// 模拟建筑名称和ID
// ============================================================================
static const TArray<FString> BuildingNames = {
	TEXT("A座·研发中心"),
	TEXT("B座·数据中心"),
	TEXT("C座·行政楼"),
	TEXT("D座·会议中心"),
	TEXT("E座·员工公寓"),
};

static const TArray<FString> BuildingIDs = {
	TEXT("BLDG_A"),
	TEXT("BLDG_B"),
	TEXT("BLDG_C"),
	TEXT("BLDG_D"),
	TEXT("BLDG_E"),
};

// ============================================================================
UDTPMockDataProvider::UDTPMockDataProvider()
{
	RandomStream.GenerateNewSeed();
}

void UDTPMockDataProvider::StartFetching()
{
	bIsFetching = true;

	// 高频：建筑人数/能耗、交通流量（保留实时跳动感）
	GenerateMockBuildingData();
	GenerateMockTrafficData();

	// 低频：天气、告警（变化太频繁会让场景天气和告警弹窗乱跳）
	const double Now = FPlatformTime::Seconds();
	if (LastSlowDataRefreshTime <= 0.0 || Now - LastSlowDataRefreshTime >= SlowDataRefreshInterval)
	{
		GenerateMockWeatherData();
		GenerateMockAlerts();
		LastSlowDataRefreshTime = Now;
	}

	OnDataUpdated.Broadcast();

	UE_LOG(LogDTP, Log, TEXT("[DTP] MockDataProvider 开始生成模拟数据"));
}

void UDTPMockDataProvider::StopFetching()
{
	bIsFetching = false;
	UE_LOG(LogDTP, Log, TEXT("[DTP] MockDataProvider 停止"));
}

void UDTPMockDataProvider::SetUpdateInterval(float Seconds)
{
	UpdateInterval = FMath::Max(0.5f, Seconds);
}

TArray<FDTPBuildingData> UDTPMockDataProvider::GetBuildingData() const
{
	return CachedBuildingData;
}

FDTPWeatherData UDTPMockDataProvider::GetWeatherData() const
{
	return CachedWeatherData;
}

FDTPTrafficData UDTPMockDataProvider::GetTrafficData() const
{
	return CachedTrafficData;
}

TArray<FDTPAlertData> UDTPMockDataProvider::GetAlertData() const
{
	return CachedAlertData;
}

// ============================================================================
// 数据生成
// ============================================================================

void UDTPMockDataProvider::GenerateMockBuildingData()
{
	CachedBuildingData.Empty();

	for (int32 i = 0; i < BuildingNames.Num(); ++i)
	{
		FDTPBuildingData Building;
		Building.BuildingId = BuildingIDs[i];
		Building.BuildingName = BuildingNames[i];
		Building.FloorCount = FMath::RandRange(3, 12);
		Building.Area = FMath::RandRange(2000.0f, 15000.0f);
		Building.Occupancy = FMath::RandRange(50, 500);
		Building.PowerUsage = FMath::FRandRange(100.0f, 800.0f);
		Building.WaterUsage = FMath::FRandRange(10.0f, 100.0f);

		// 每栋建筑生成几个IoT设备
		Building.Devices = {
			GenerateDeviceData(Building.BuildingId, TEXT("TEMP_01"), EDTPDeviceType::Temperature, 18.0f, 35.0f, TEXT("°C")),
			GenerateDeviceData(Building.BuildingId, TEXT("HUM_01"),  EDTPDeviceType::Humidity, 30.0f, 80.0f, TEXT("%")),
			GenerateDeviceData(Building.BuildingId, TEXT("PWR_01"),  EDTPDeviceType::Power, 0.0f, 1000.0f, TEXT("kW")),
			GenerateDeviceData(Building.BuildingId, TEXT("WTR_01"),  EDTPDeviceType::Water, 0.0f, 200.0f, TEXT("m³/h")),
			GenerateDeviceData(Building.BuildingId, TEXT("OCC_01"),  EDTPDeviceType::Occupancy, 0.0f, 600.0f, TEXT("人")),
		};

		CachedBuildingData.Add(Building);
	}
}

void UDTPMockDataProvider::GenerateMockWeatherData()
{
	CachedWeatherData.WeatherType = static_cast<EDTPWeatherType>(FMath::RandRange(0, 3));
	CachedWeatherData.Temperature = FMath::FRandRange(15.0f, 38.0f);
	CachedWeatherData.Humidity = FMath::FRandRange(30.0f, 90.0f);
	CachedWeatherData.WindSpeed = FMath::FRandRange(0.0f, 15.0f);
	CachedWeatherData.AQI = FMath::RandRange(20, 150);
}

void UDTPMockDataProvider::GenerateMockTrafficData()
{
	CachedTrafficData.VehicleCount = FMath::RandRange(0, 200);
	CachedTrafficData.AverageSpeed = FMath::FRandRange(20.0f, 60.0f);

	if (CachedTrafficData.AverageSpeed > 40.0f)
		CachedTrafficData.CongestionLevel = TEXT("畅通");
	else if (CachedTrafficData.AverageSpeed > 20.0f)
		CachedTrafficData.CongestionLevel = TEXT("缓行");
	else
		CachedTrafficData.CongestionLevel = TEXT("拥堵");
}

void UDTPMockDataProvider::GenerateMockAlerts()
{
	CachedAlertData.Empty();

	// 随机生成0-2条告警
	const int32 AlertCount = FMath::RandRange(0, 2);
	for (int32 i = 0; i < AlertCount; ++i)
	{
		FDTPAlertData Alert;
		Alert.AlertId = FString::Printf(TEXT("ALT_%d"), FMath::Rand());
		Alert.Level = static_cast<EDTPAlertLevel>(FMath::RandRange(0, 2));
		Alert.Timestamp = FDateTime::Now();
		Alert.bIsAcknowledged = false;

		const int32 BuildingIdx = FMath::RandRange(0, BuildingIDs.Num() - 1);
		Alert.BuildingId = BuildingIDs[BuildingIdx];

		switch (Alert.Level)
		{
		case EDTPAlertLevel::Info:
			Alert.Title = TEXT("设备状态更新");
			Alert.Message = FString::Printf(TEXT("%s 设备固件升级完成"), *BuildingNames[BuildingIdx]);
			break;
		case EDTPAlertLevel::Warning:
			Alert.Title = TEXT("能耗异常");
			Alert.Message = FString::Printf(TEXT("%s 今日用电量超过日均值20%%"), *BuildingNames[BuildingIdx]);
			break;
		case EDTPAlertLevel::Critical:
			Alert.Title = TEXT("消防告警");
			Alert.Message = FString::Printf(TEXT("%s 3层烟雾传感器触发"), *BuildingNames[BuildingIdx]);
			break;
		}

		CachedAlertData.Add(Alert);
	}
}

// ============================================================================
// 辅助函数
// ============================================================================

FDTPDeviceData UDTPMockDataProvider::GenerateDeviceData(
	const FString& BuildingId, const FString& DeviceSuffix,
	EDTPDeviceType Type, float MinVal, float MaxVal, const FString& Unit)
{
	FDTPDeviceData Data;
	Data.DeviceId = FString::Printf(TEXT("%s_%s"), *BuildingId, *DeviceSuffix);
	Data.DeviceName = DeviceSuffix;
	Data.Type = Type;
	Data.MinValue = MinVal;
	Data.MaxValue = MaxVal;
	Data.Unit = Unit;
	Data.CurrentValue = FMath::FRandRange(MinVal, MaxVal);
	Data.Timestamp = FDateTime::Now();
	Data.bIsAlert = (Type == EDTPDeviceType::FireAlarm && FMath::FRand() < 0.05f);
	return Data;
}