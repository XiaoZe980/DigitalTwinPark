// DigitalTwinPark - 智慧园区数字孪生
// 数据类型定义 - 所有数据结构和枚举

#pragma once

#include "CoreMinimal.h"
#include "DTPDataTypes.generated.h"

// ============================================================================
// 枚举
// ============================================================================

/** IoT 设备类型 */
UENUM(BlueprintType)
enum class EDTPDeviceType : uint8
{
	Temperature		UMETA(DisplayName = "温度"),
	Humidity		UMETA(DisplayName = "湿度"),
	Power			UMETA(DisplayName = "电量"),
	Water			UMETA(DisplayName = "水量"),
	Occupancy		UMETA(DisplayName = "人流量"),
	Elevator		UMETA(DisplayName = "电梯"),
	FireAlarm		UMETA(DisplayName = "消防"),
	Security		UMETA(DisplayName = "安防"),
};

/** 数据提供者类型 */
UENUM(BlueprintType)
enum class EDTPDataProviderType : uint8
{
	Mock	UMETA(DisplayName = "模拟数据"),
	HTTP	UMETA(DisplayName = "HTTP接口"),
};

/** 天气类型 */
UENUM(BlueprintType)
enum class EDTPWeatherType : uint8
{
	Sunny		UMETA(DisplayName = "晴天"),
	Cloudy		UMETA(DisplayName = "多云"),
	Rainy		UMETA(DisplayName = "雨天"),
	Snowy		UMETA(DisplayName = "雪天"),
};

/** 告警级别 */
UENUM(BlueprintType)
enum class EDTPAlertLevel : uint8
{
	Info	UMETA(DisplayName = "信息"),
	Warning	UMETA(DisplayName = "警告"),
	Critical UMETA(DisplayName = "严重"),
};

// ============================================================================
// 数据结构
// ============================================================================

/** IoT 设备数据点 */
USTRUCT(BlueprintType)
struct FDTPDeviceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	FString DeviceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	FString DeviceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	EDTPDeviceType Type = EDTPDeviceType::Temperature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	float CurrentValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	float MaxValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	FString Unit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	bool bIsAlert = false;
};

/** 建筑汇总数据 */
USTRUCT(BlueprintType)
struct FDTPBuildingData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	FString BuildingId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	FString BuildingName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	int32 FloorCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	float Area = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	int32 Occupancy = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	float PowerUsage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	float WaterUsage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	TArray<FDTPDeviceData> Devices;
};

/** 天气数据 */
USTRUCT(BlueprintType)
struct FDTPWeatherData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	EDTPWeatherType WeatherType = EDTPWeatherType::Sunny;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	float Temperature = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	float Humidity = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	float WindSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	int32 AQI = 50;
};

/** 交通数据 */
USTRUCT(BlueprintType)
struct FDTPTrafficData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	int32 VehicleCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	float AverageSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	FString CongestionLevel; // "畅通", "缓行", "拥堵"
};

/** 告警事件 */
USTRUCT(BlueprintType)
struct FDTPAlertData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	FString AlertId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	EDTPAlertLevel Level = EDTPAlertLevel::Info;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	FString Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	FString BuildingId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Data")
	bool bIsAcknowledged = false;
};