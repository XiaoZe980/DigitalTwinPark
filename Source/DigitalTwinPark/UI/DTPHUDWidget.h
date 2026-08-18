// DigitalTwinPark - 智慧园区数字孪生
// 主HUD Widget - 管理所有UI子面板

#pragma once

#include "CoreMinimal.h"
#include "DTPBaseWidget.h"
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