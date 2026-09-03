// DigitalTwinPark - 智慧园区数字孪生
// 主HUD Widget - 管理所有UI子面板

#pragma once

#include "CoreMinimal.h"
#include "DTPBaseWidget.h"
#include "Data/DTPDataTypes.h"
#include "DTPHUDWidget.generated.h"

/**
 * 主HUD容器
 * 管理顶栏、数据面板、告警弹窗等子Widget
 * 大部分子Widget通过蓝图BindWidget绑定
 */
UCLASS(abstract)
class DIGITALTWINPARK_API UDTPHUDWidget : public UDTPBaseWidget
{
	GENERATED_BODY()

public:
	virtual void InitializeWidget() override;
	virtual void RefreshData() override;

	/** 显示/隐藏数据面板 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|UI")
	void ToggleDataPanel();

	/** 格式化天气显示文本（类型转中文 + 温度 + AQI） */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|UI")
	FString FormatWeatherText(const struct FDTPWeatherData& Weather) const;

	/** 格式化当前时间文本 HH:MM:SS */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|UI")
	FString GetCurrentTimeText() const;

	/** 告警级别 → 边框颜色（Info蓝 / Warning黄 / Critical红） */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|UI")
	FLinearColor GetAlertColor(const EDTPAlertLevel Level) const;

	// ========================================================================
	// 园区总览汇总（遍历建筑数据实时求和，供大屏仪表盘绑定）
	// ========================================================================

	/** 园区总人数 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|UI")
	int32 GetTotalOccupancy() const;

	/** 园区总能耗 (kW) */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|UI")
	float GetTotalPowerUsage() const;

	/** 园区总用水 (m³) */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|UI")
	float GetTotalWaterUsage() const;

	/** 活跃告警数 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|UI")
	int32 GetActiveAlertCount() const;

	/** 显示告警 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DigitalTwinPark|UI")
	void ShowAlert(const FString& Title, const FString& Message, int32 Level);

	/** 显示建筑信息 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DigitalTwinPark|UI")
	void ShowBuildingInfo(const struct FDTPBuildingData& BuildingData);

	/** 隐藏建筑信息 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DigitalTwinPark|UI")
	void HideBuildingInfo();

protected:
	/** 数据面板是否可见 */
	bool bDataPanelVisible = false;
};