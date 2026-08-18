// DigitalTwinPark - 智慧园区数字孪生
// 数据子系统 - 统一数据管理层，缓存+分发

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DTPDataTypes.h"
#include "DTPDataSubsystem.generated.h"

class IDTPDataProvider;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDTPOnBuildingDataUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDTPOnWeatherDataUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDTPOnTrafficDataUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDTPOnAlertDataUpdated);

/**
 * 数据子系统
 * 作为GameInstanceSubsystem，全局唯一，生命周期跟随游戏
 * 负责：
 * 1. 管理数据提供者（Mock/HTTP切换）
 * 2. 定时拉取数据
 * 3. 缓存最新数据
 * 4. 广播数据变化通知
 */
UCLASS()
class DIGITALTWINPARK_API UDTPDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ========================================================================
	// Subsystem生命周期
	// ========================================================================
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// 数据源切换
	// ========================================================================

	/** 切换到模拟数据 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Data")
	void UseMockData();

	/** 切换到HTTP数据源 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Data")
	void UseHttpData(const FString& BaseURL = TEXT("http://localhost:8080/api"));

	/** 设置数据更新间隔（秒） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Data")
	void SetUpdateInterval(float Seconds);

	// ========================================================================
	// 数据获取（从缓存读取，非阻塞）
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Data")
	const TArray<FDTPBuildingData>& GetBuildingData() const;

	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Data")
	const FDTPWeatherData& GetWeatherData() const;

	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Data")
	const FDTPTrafficData& GetTrafficData() const;

	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Data")
	const TArray<FDTPAlertData>& GetAlertData() const;

	/** 根据ID查找建筑 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Data")
	bool FindBuildingById(const FString& BuildingId, FDTPBuildingData& OutData) const;

	// ========================================================================
	// 数据更新委托（UI绑定这些来响应数据变化）
	// ========================================================================

	UPROPERTY(BlueprintAssignable, Category = "DigitalTwinPark|Data")
	FDTPOnBuildingDataUpdated OnBuildingDataUpdated;

	UPROPERTY(BlueprintAssignable, Category = "DigitalTwinPark|Data")
	FDTPOnWeatherDataUpdated OnWeatherDataUpdated;

	UPROPERTY(BlueprintAssignable, Category = "DigitalTwinPark|Data")
	FDTPOnTrafficDataUpdated OnTrafficDataUpdated;

	UPROPERTY(BlueprintAssignable, Category = "DigitalTwinPark|Data")
	FDTPOnAlertDataUpdated OnAlertDataUpdated;

private:
	/** 定时更新回调 */
	void OnUpdateTimer();

	/** 从Provider拉取所有数据到缓存 */
	void RefreshAllData();

	/** 数据提供者 */
	UPROPERTY()
	TObjectPtr<UObject> DataProviderObject;

	// 数据提供者接口（缓存引用）
	IDTPDataProvider* DataProvider = nullptr;

	// 缓存数据
	TArray<FDTPBuildingData> CachedBuildingData;
	FDTPWeatherData CachedWeatherData;
	FDTPTrafficData CachedTrafficData;
	TArray<FDTPAlertData> CachedAlertData;

	// 定时器句柄
	FTimerHandle UpdateTimerHandle;

	// 更新间隔
	float UpdateInterval = 2.0f;
};