// DigitalTwinPark - 智慧园区数字孪生
// 建筑数据浮动标签 - 建筑头顶常显的名称与关键指标

#pragma once

#include "CoreMinimal.h"
#include "DTPBaseWidget.h"
#include "Data/DTPDataTypes.h"
#include "DTPBuildingLabelWidget.generated.h"

/**
 * 建筑数据浮动标签
 * 挂在 ADTPBuildingActor 的 InfoWidget 上，常显在建筑头顶
 * 蓝图子类实现 UpdateLabelData 来填充文本
 */
UCLASS(abstract)
class DIGITALTWINPARK_API UDTPBuildingLabelWidget : public UDTPBaseWidget
{
	GENERATED_BODY()

public:
	/** 数据更新时刷新标签内容（蓝图实现：填名称/人数/能耗等） */
	UFUNCTION(BlueprintImplementableEvent, Category = "DigitalTwinPark|UI")
	void UpdateLabelData(const FDTPBuildingData& BuildingData);

	/** 一行摘要文本："研发大楼  370人  380kW" */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|UI")
	FString FormatLabelSummary(const FDTPBuildingData& BuildingData) const;
};
