// DigitalTwinPark - 智慧园区数字孪生
// 建筑数据浮动标签实现

#include "DTPBuildingLabelWidget.h"

FString UDTPBuildingLabelWidget::FormatLabelSummary(const FDTPBuildingData& BuildingData) const
{
	// 建筑名 + 人数 + 能耗一行摘要，供蓝图直接绑定到 TextBlock
	return FString::Printf(
		TEXT("%s  %d人  %.0fkW"),
		*BuildingData.BuildingName,
		BuildingData.Occupancy,
		BuildingData.PowerUsage
	);
}
