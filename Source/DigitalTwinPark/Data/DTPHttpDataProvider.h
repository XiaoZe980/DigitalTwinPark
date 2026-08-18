// DigitalTwinPark - 智慧园区数字孪生
// HTTP数据提供者 - 通过REST API获取真实IoT数据

#pragma once

#include "CoreMinimal.h"
#include "DTPDataProvider.h"
#include "Interfaces/IHttpRequest.h"
#include "DTPHttpDataProvider.generated.h"

/**
 * HTTP数据提供者
 * 通过HTTP REST API获取真实IoT传感器数据
 * 接口约定：
 *   GET /api/buildings     -> [{...}]  建筑列表+设备数据
 *   GET /api/weather       -> {...}    天气数据
 *   GET /api/traffic       -> {...}    交通数据
 *   GET /api/alerts        -> [{...}]  告警列表
 */
UCLASS(BlueprintType, Blueprintable)
class DIGITALTWINPARK_API UDTPHttpDataProvider : public UObject, public IDTPDataProvider
{
	GENERATED_BODY()

public:
	UDTPHttpDataProvider();

	// ~IDTPDataProvider 接口实现
	virtual void StartFetching() override;
	virtual void StopFetching() override;
	virtual void SetUpdateInterval(float Seconds) override;
	virtual TArray<FDTPBuildingData> GetBuildingData() const override;
	virtual FDTPWeatherData GetWeatherData() const override;
	virtual FDTPTrafficData GetTrafficData() const override;
	virtual TArray<FDTPAlertData> GetAlertData() const override;

	/** 设置API基础URL */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Data")
	void SetBaseURL(const FString& URL);

private:
	/** 发送HTTP请求 */
	void FetchAllData();

	/** HTTP响应回调 */
	void OnBuildingsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnWeatherResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnTrafficResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnAlertsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** JSON解析 */
	void ParseBuildingsJson(const FString& JsonString);
	void ParseWeatherJson(const FString& JsonString);
	void ParseTrafficJson(const FString& JsonString);
	void ParseAlertsJson(const FString& JsonString);

	/** 检查是否所有请求完成 */
	void CheckAllRequestsComplete();

	// API配置
	FString BaseURL = TEXT("http://localhost:8080/api");

	// 缓存数据
	TArray<FDTPBuildingData> CachedBuildingData;
	FDTPWeatherData CachedWeatherData;
	FDTPTrafficData CachedTrafficData;
	TArray<FDTPAlertData> CachedAlertData;

	// 请求状态
	int32 PendingRequests = 0;
	float UpdateInterval = 5.0f;
	bool bIsFetching = false;
};