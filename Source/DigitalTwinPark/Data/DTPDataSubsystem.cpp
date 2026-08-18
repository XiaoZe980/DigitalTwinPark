// DigitalTwinPark - 智慧园区数字孪生
// 数据子系统实现

#include "DTPDataSubsystem.h"
#include "DTPDataProvider.h"
#include "DTPMockDataProvider.h"
#include "DTPHttpDataProvider.h"
#include "DigitalTwinPark.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UDTPDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogDTP, Log, TEXT("[DTP] DataSubsystem 初始化"));

	// 默认使用模拟数据
	UseMockData();
}

void UDTPDataSubsystem::Deinitialize()
{
	// 停止定时器
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	// 停止数据提供者
	if (DataProvider)
	{
		DataProvider->StopFetching();
	}

	Super::Deinitialize();
}

// ============================================================================
// 数据源切换
// ============================================================================

void UDTPDataSubsystem::UseMockData()
{
	// 停止旧提供者
	if (DataProvider)
	{
		DataProvider->StopFetching();
	}

	// 创建模拟数据提供者
	UDTPMockDataProvider* MockProvider = NewObject<UDTPMockDataProvider>(this);
	MockProvider->SetUpdateInterval(UpdateInterval);
	DataProviderObject = MockProvider;
	DataProvider = MockProvider;

	// 绑定数据更新回调
	DataProvider->OnDataUpdated.AddUObject(this, &UDTPDataSubsystem::RefreshAllData);

	// 启动
	DataProvider->StartFetching();

	UE_LOG(LogDTP, Log, TEXT("[DTP] 已切换到模拟数据源"));
}

void UDTPDataSubsystem::UseHttpData(const FString& BaseURL)
{
	// 停止旧提供者
	if (DataProvider)
	{
		DataProvider->StopFetching();
	}

	// 创建HTTP数据提供者
	UDTPHttpDataProvider* HttpProvider = NewObject<UDTPHttpDataProvider>(this);
	HttpProvider->SetBaseURL(BaseURL);
	HttpProvider->SetUpdateInterval(UpdateInterval);
	DataProviderObject = HttpProvider;
	DataProvider = HttpProvider;

	// 绑定数据更新回调
	DataProvider->OnDataUpdated.AddUObject(this, &UDTPDataSubsystem::RefreshAllData);

	// 启动
	DataProvider->StartFetching();

	UE_LOG(LogDTP, Log, TEXT("[DTP] 已切换到HTTP数据源: %s"), *BaseURL);
}

void UDTPDataSubsystem::SetUpdateInterval(float Seconds)
{
	UpdateInterval = FMath::Max(0.5f, Seconds);
	if (DataProvider)
	{
		DataProvider->SetUpdateInterval(UpdateInterval);
	}
}

// ============================================================================
// 数据获取
// ============================================================================

const TArray<FDTPBuildingData>& UDTPDataSubsystem::GetBuildingData() const
{
	return CachedBuildingData;
}

const FDTPWeatherData& UDTPDataSubsystem::GetWeatherData() const
{
	return CachedWeatherData;
}

const FDTPTrafficData& UDTPDataSubsystem::GetTrafficData() const
{
	return CachedTrafficData;
}

const TArray<FDTPAlertData>& UDTPDataSubsystem::GetAlertData() const
{
	return CachedAlertData;
}

bool UDTPDataSubsystem::FindBuildingById(const FString& BuildingId, FDTPBuildingData& OutData) const
{
	for (const auto& Building : CachedBuildingData)
	{
		if (Building.BuildingId == BuildingId)
		{
			OutData = Building;
			return true;
		}
	}
	return false;
}

// ============================================================================
// 内部函数
// ============================================================================

void UDTPDataSubsystem::RefreshAllData()
{
	if (!DataProvider) return;

	// 拉取所有数据到缓存
	CachedBuildingData = DataProvider->GetBuildingData();
	CachedWeatherData = DataProvider->GetWeatherData();
	CachedTrafficData = DataProvider->GetTrafficData();
	CachedAlertData = DataProvider->GetAlertData();

	// 广播数据更新
	OnBuildingDataUpdated.Broadcast();
	OnWeatherDataUpdated.Broadcast();
	OnTrafficDataUpdated.Broadcast();
	OnAlertDataUpdated.Broadcast();
}

void UDTPDataSubsystem::OnUpdateTimer()
{
	if (DataProvider)
	{
		DataProvider->StartFetching(); // 触发新一轮数据拉取
	}
}