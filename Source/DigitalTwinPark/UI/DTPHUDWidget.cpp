// DigitalTwinPark - 智慧园区数字孪生
// 主HUD Widget实现

#include "DTPHUDWidget.h"
#include "DigitalTwinPark.h"
#include "Data/DTPDataSubsystem.h"

void UDTPHUDWidget::InitializeWidget()
{
	Super::InitializeWidget();
	UE_LOG(LogDTP, Log, TEXT("[DTP] HUDWidget 初始化"));
}

void UDTPHUDWidget::RefreshData()
{
	Super::RefreshData();
	// 数据刷新由蓝图的Event Tick或数据绑定处理
}

void UDTPHUDWidget::ToggleDataPanel()
{
	bDataPanelVisible = !bDataPanelVisible;
	// 蓝图实现UI动画
}

FString UDTPHUDWidget::FormatWeatherText(const FDTPWeatherData& Weather) const
{
	FString WeatherName;
	switch (Weather.WeatherType)
	{
	case EDTPWeatherType::Sunny:  WeatherName = TEXT("晴");   break;
	case EDTPWeatherType::Cloudy: WeatherName = TEXT("多云"); break;
	case EDTPWeatherType::Rainy:  WeatherName = TEXT("雨");   break;
	case EDTPWeatherType::Snowy:  WeatherName = TEXT("雪");   break;
	default: WeatherName = TEXT("--"); break;
	}
	return FString::Printf(
		TEXT("%s %d°C AQI %d"),
		*WeatherName,
		FMath::RoundToInt(Weather.Temperature),
		Weather.AQI
	);
}

FString UDTPHUDWidget::GetCurrentTimeText() const
{
	const FDateTime Now = FDateTime::Now();
	return FString::Printf(
		TEXT("%02d:%02d:%02d"),
		Now.GetHour(), Now.GetMinute(), Now.GetSecond()
	);
}

FLinearColor UDTPHUDWidget::GetAlertColor(const EDTPAlertLevel Level) const
{
	switch (Level)
	{
	case EDTPAlertLevel::Info:     return FLinearColor(0.2f, 0.4f, 0.9f, 1.0f); // 蓝
	case EDTPAlertLevel::Warning:  return FLinearColor(0.9f, 0.7f, 0.1f, 1.0f); // 黄
	case EDTPAlertLevel::Critical: return FLinearColor(0.9f, 0.2f, 0.1f, 1.0f); // 红
	default: return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // 灰兜底
	}
}

int32 UDTPHUDWidget::GetTotalOccupancy() const
{
	UDTPDataSubsystem* Data = GetDataSubsystem();
	if (!Data) return 0;
	int32 Total = 0;
	for (const FDTPBuildingData& B : Data->GetBuildingData())
	{
		Total += B.Occupancy;
	}
	return Total;
}

float UDTPHUDWidget::GetTotalPowerUsage() const
{
	UDTPDataSubsystem* Data = GetDataSubsystem();
	if (!Data) return 0.0f;
	float Total = 0.0f;
	for (const FDTPBuildingData& B : Data->GetBuildingData())
	{
		Total += B.PowerUsage;
	}
	return Total;
}

float UDTPHUDWidget::GetTotalWaterUsage() const
{
	UDTPDataSubsystem* Data = GetDataSubsystem();
	if (!Data) return 0.0f;
	float Total = 0.0f;
	for (const FDTPBuildingData& B : Data->GetBuildingData())
	{
		Total += B.WaterUsage;
	}
	return Total;
}

int32 UDTPHUDWidget::GetActiveAlertCount() const
{
	UDTPDataSubsystem* Data = GetDataSubsystem();
	if (!Data) return 0;
	return Data->GetAlertData().Num();
}