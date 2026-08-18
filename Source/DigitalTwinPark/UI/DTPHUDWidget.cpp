// DigitalTwinPark - 智慧园区数字孪生
// 主HUD Widget实现

#include "DTPHUDWidget.h"
#include "DigitalTwinPark.h"

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