// DigitalTwinPark - 智慧园区数字孪生
// HTTP数据提供者实现

#include "DTPHttpDataProvider.h"
#include "DigitalTwinPark.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"

UDTPHttpDataProvider::UDTPHttpDataProvider()
{
}

void UDTPHttpDataProvider::SetBaseURL(const FString& URL)
{
	BaseURL = URL;
	UE_LOG(LogDTP, Log, TEXT("[DTP] HTTP数据源URL: %s"), *BaseURL);
}

void UDTPHttpDataProvider::StartFetching()
{
	bIsFetching = true;
	FetchAllData();
	UE_LOG(LogDTP, Log, TEXT("[DTP] HttpDataProvider 开始拉取数据"));
}

void UDTPHttpDataProvider::StopFetching()
{
	bIsFetching = false;
}

void UDTPHttpDataProvider::SetUpdateInterval(float Seconds)
{
	UpdateInterval = FMath::Max(1.0f, Seconds);
}

TArray<FDTPBuildingData> UDTPHttpDataProvider::GetBuildingData() const
{
	return CachedBuildingData;
}

FDTPWeatherData UDTPHttpDataProvider::GetWeatherData() const
{
	return CachedWeatherData;
}

FDTPTrafficData UDTPHttpDataProvider::GetTrafficData() const
{
	return CachedTrafficData;
}

TArray<FDTPAlertData> UDTPHttpDataProvider::GetAlertData() const
{
	return CachedAlertData;
}

// ============================================================================
// HTTP请求
// ============================================================================

void UDTPHttpDataProvider::FetchAllData()
{
	if (!bIsFetching) return;

	FHttpModule& HttpModule = FHttpModule::Get();
	// 配置了真实天气Key时，天气 + 空气质量各发一个请求到和风
	bUseRealWeather = !WeatherAPIKey.IsEmpty() && !WeatherLocation.IsEmpty();
	PendingRequests = bUseRealWeather ? 5 : 4;

	// 请求建筑数据
	{
		TSharedRef<IHttpRequest> Request = HttpModule.CreateRequest();
		Request->SetURL(BaseURL + TEXT("/buildings"));
		Request->SetVerb(TEXT("GET"));
		Request->OnProcessRequestComplete().BindUObject(this, &UDTPHttpDataProvider::OnBuildingsResponse);
		Request->ProcessRequest();
	}

	// 请求天气数据（配置真实天气源则请求和风实况API，否则走 BaseURL 假数据）
	const FString WeatherHost = WeatherAPIHost.IsEmpty() ? TEXT("https://devapi.qweather.com") : WeatherAPIHost;
	{
		TSharedRef<IHttpRequest> Request = HttpModule.CreateRequest();
		if (bUseRealWeather)
		{
			Request->SetURL(FString::Printf(
				TEXT("%s/v7/weather/now?location=%s&key=%s"),
				*WeatherHost, *WeatherLocation, *WeatherAPIKey));
		}
		else
		{
			Request->SetURL(BaseURL + TEXT("/weather"));
		}
		Request->SetVerb(TEXT("GET"));
		Request->OnProcessRequestComplete().BindUObject(this, &UDTPHttpDataProvider::OnWeatherResponse);
		Request->ProcessRequest();
	}

	// 请求空气质量（真实天气源时，用于填 AQI）
	if (bUseRealWeather)
	{
		TSharedRef<IHttpRequest> Request = HttpModule.CreateRequest();
		Request->SetURL(FString::Printf(
			TEXT("%s/v7/air/now?location=%s&key=%s"),
			*WeatherHost, *WeatherLocation, *WeatherAPIKey));
		Request->SetVerb(TEXT("GET"));
		Request->OnProcessRequestComplete().BindUObject(this, &UDTPHttpDataProvider::OnAirResponse);
		Request->ProcessRequest();
	}

	// 请求交通数据
	{
		TSharedRef<IHttpRequest> Request = HttpModule.CreateRequest();
		Request->SetURL(BaseURL + TEXT("/traffic"));
		Request->SetVerb(TEXT("GET"));
		Request->OnProcessRequestComplete().BindUObject(this, &UDTPHttpDataProvider::OnTrafficResponse);
		Request->ProcessRequest();
	}

	// 请求告警数据
	{
		TSharedRef<IHttpRequest> Request = HttpModule.CreateRequest();
		Request->SetURL(BaseURL + TEXT("/alerts"));
		Request->SetVerb(TEXT("GET"));
		Request->OnProcessRequestComplete().BindUObject(this, &UDTPHttpDataProvider::OnAlertsResponse);
		Request->ProcessRequest();
	}
}

// ============================================================================
// HTTP响应回调
// ============================================================================

void UDTPHttpDataProvider::OnBuildingsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		ParseBuildingsJson(Response->GetContentAsString());
	}
	else
	{
		UE_LOG(LogDTP, Warning, TEXT("[DTP] 建筑数据请求失败"));
	}
	CheckAllRequestsComplete();
}

void UDTPHttpDataProvider::OnWeatherResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		ParseWeatherJson(Response->GetContentAsString());
	}
	else
	{
		UE_LOG(LogDTP, Warning, TEXT("[DTP] 天气数据请求失败"));
	}
	CheckAllRequestsComplete();
}

void UDTPHttpDataProvider::OnTrafficResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		ParseTrafficJson(Response->GetContentAsString());
	}
	else
	{
		UE_LOG(LogDTP, Warning, TEXT("[DTP] 交通数据请求失败"));
	}
	CheckAllRequestsComplete();
}

void UDTPHttpDataProvider::OnAlertsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		ParseAlertsJson(Response->GetContentAsString());
	}
	else
	{
		UE_LOG(LogDTP, Warning, TEXT("[DTP] 告警数据请求失败"));
	}
	CheckAllRequestsComplete();
}

void UDTPHttpDataProvider::OnAirResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		ParseAirJson(Response->GetContentAsString());
	}
	else
	{
		UE_LOG(LogDTP, Warning, TEXT("[DTP] 空气质量数据请求失败"));
	}
	CheckAllRequestsComplete();
}

void UDTPHttpDataProvider::CheckAllRequestsComplete()
{
	PendingRequests--;
	if (PendingRequests <= 0)
	{
		// 所有请求完成，广播数据更新
		OnDataUpdated.Broadcast();
	}
}

// ============================================================================
// JSON解析
// ============================================================================

void UDTPHttpDataProvider::ParseBuildingsJson(const FString& JsonString)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		// 尝试用FJsonObjectConverter直接解析结构体数组
		const TArray<TSharedPtr<FJsonValue>>* BuildingsArray;
		if (JsonObject->TryGetArrayField(TEXT("data"), BuildingsArray))
		{
			CachedBuildingData.Empty();
			for (const auto& Value : *BuildingsArray)
			{
				FDTPBuildingData Building;
				if (FJsonObjectConverter::JsonObjectToUStruct(
					Value->AsObject().ToSharedRef(), &Building, 0, 0))
				{
					CachedBuildingData.Add(Building);
				}
			}
		}
		// 兼容直接返回数组的格式
		else if (JsonObject->TryGetArrayField(TEXT("buildings"), BuildingsArray))
		{
			CachedBuildingData.Empty();
			for (const auto& Value : *BuildingsArray)
			{
				FDTPBuildingData Building;
				if (FJsonObjectConverter::JsonObjectToUStruct(
					Value->AsObject().ToSharedRef(), &Building, 0, 0))
				{
					CachedBuildingData.Add(Building);
				}
			}
		}
	}
}

void UDTPHttpDataProvider::ParseWeatherJson(const FString& JsonString)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return;
	}

	if (bUseRealWeather)
	{
		// 和风天气: {"code":"200","now":{"temp":"28","text":"晴","humidity":"60","windSpeed":"9"}}
		const TSharedPtr<FJsonObject>* NowObj;
		if (JsonObject->TryGetObjectField(TEXT("now"), NowObj))
		{
			FString Text, TempStr, HumidityStr, WindStr;
			(*NowObj)->TryGetStringField(TEXT("text"), Text);
			(*NowObj)->TryGetStringField(TEXT("temp"), TempStr);
			(*NowObj)->TryGetStringField(TEXT("humidity"), HumidityStr);
			(*NowObj)->TryGetStringField(TEXT("windSpeed"), WindStr);

			CachedWeatherData.WeatherType = MapWeatherText(Text);
			CachedWeatherData.Temperature = FCString::Atof(*TempStr);
			CachedWeatherData.Humidity = FCString::Atof(*HumidityStr);
			CachedWeatherData.WindSpeed = FCString::Atof(*WindStr);
			// AQI 由 ParseAirJson 单独填充
		}
	}
	else
	{
		// 本地测试服务器约定: {"data":{...}}
		const TSharedPtr<FJsonObject>* DataObj;
		if (JsonObject->TryGetObjectField(TEXT("data"), DataObj))
		{
			FJsonObjectConverter::JsonObjectToUStruct(
				DataObj->ToSharedRef(), &CachedWeatherData, 0, 0);
		}
	}
}

void UDTPHttpDataProvider::ParseTrafficJson(const FString& JsonString)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		const TSharedPtr<FJsonObject>* DataObj;
		if (JsonObject->TryGetObjectField(TEXT("data"), DataObj))
		{
			FJsonObjectConverter::JsonObjectToUStruct(
				DataObj->ToSharedRef(), &CachedTrafficData, 0, 0);
		}
	}
}

void UDTPHttpDataProvider::ParseAlertsJson(const FString& JsonString)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		const TArray<TSharedPtr<FJsonValue>>* AlertsArray;
		if (JsonObject->TryGetArrayField(TEXT("data"), AlertsArray))
		{
			CachedAlertData.Empty();
			for (const auto& Value : *AlertsArray)
			{
				FDTPAlertData Alert;
				if (FJsonObjectConverter::JsonObjectToUStruct(
					Value->AsObject().ToSharedRef(), &Alert, 0, 0))
				{
					CachedAlertData.Add(Alert);
				}
			}
		}
	}
}

void UDTPHttpDataProvider::ParseAirJson(const FString& JsonString)
{
	// 和风空气质量: {"code":"200","now":{"aqi":"52",...}}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		const TSharedPtr<FJsonObject>* NowObj;
		if (JsonObject->TryGetObjectField(TEXT("now"), NowObj))
		{
			FString AqiStr;
			if ((*NowObj)->TryGetStringField(TEXT("aqi"), AqiStr))
			{
				CachedWeatherData.AQI = FCString::Atoi(*AqiStr);
			}
		}
	}
}

EDTPWeatherType UDTPHttpDataProvider::MapWeatherText(const FString& Text) const
{
	if (Text.Contains(TEXT("雨"))) return EDTPWeatherType::Rainy;
	if (Text.Contains(TEXT("雪"))) return EDTPWeatherType::Snowy;
	if (Text.Contains(TEXT("晴"))) return EDTPWeatherType::Sunny;
	return EDTPWeatherType::Cloudy; // 多云/阴/雾等归为多云
}

void UDTPHttpDataProvider::SetWeatherSource(const FString& APIKey, const FString& Location, const FString& Host)
{
	WeatherAPIKey = APIKey;
	WeatherLocation = Location;
	if (!Host.IsEmpty())
	{
		WeatherAPIHost = Host;
	}
	if (WeatherAPIKey.IsEmpty())
	{
		UE_LOG(LogDTP, Log, TEXT("[DTP] 真实天气源已关闭，退回本地假数据"));
	}
	else
	{
		UE_LOG(LogDTP, Log, TEXT("[DTP] 真实天气源已配置: location=%s host=%s"),
			*WeatherLocation, *WeatherAPIHost);
	}
}