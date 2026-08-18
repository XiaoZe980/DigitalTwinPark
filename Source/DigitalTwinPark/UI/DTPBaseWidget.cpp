// DigitalTwinPark - 智慧园区数字孪生
// UI基类实现

#include "DTPBaseWidget.h"
#include "Data/DTPDataSubsystem.h"
#include "DTPGameInstance.h"
#include "DigitalTwinPark.h"

void UDTPBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 获取数据子系统
	DataSubsystem = GetDataSubsystem();
}

void UDTPBaseWidget::InitializeWidget()
{
	if (bIsInitialized) return;
	bIsInitialized = true;

	// 绑定数据更新委托
	if (DataSubsystem)
	{
		DataSubsystem->OnBuildingDataUpdated.AddDynamic(this, &UDTPBaseWidget::RefreshData);
		DataSubsystem->OnWeatherDataUpdated.AddDynamic(this, &UDTPBaseWidget::RefreshData);
	}

	RefreshData();
}

void UDTPBaseWidget::RefreshData()
{
	// 子类重载此方法实现具体刷新逻辑
}

UDTPDataSubsystem* UDTPBaseWidget::GetDataSubsystem() const
{
	if (const UDTPGameInstance* GI = Cast<UDTPGameInstance>(GetGameInstance()))
	{
		return GI->GetDataSubsystem();
	}
	return nullptr;
}