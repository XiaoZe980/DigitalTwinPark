// DigitalTwinPark - 智慧园区数字孪生
// 建筑Actor实现

#include "DTPBuildingActor.h"
#include "DTPBuildingDataAsset.h"
#include "Data/DTPDataTypes.h"
#include "DigitalTwinPark.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Materials/MaterialInterface.h"

ADTPBuildingActor::ADTPBuildingActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 根组件
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootScene);

	// 建筑网格
	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	BuildingMesh->SetupAttachment(RootScene);
	BuildingMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BuildingMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// 外轮廓（运行时创建，默认隐藏）
	OutlineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OutlineMesh"));
	OutlineMesh->SetupAttachment(RootScene);
	OutlineMesh->SetVisibility(false);
	OutlineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 信息悬浮组件
	InfoWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InfoWidget"));
	InfoWidget->SetupAttachment(RootScene);
	InfoWidget->SetVisibility(false);
	InfoWidget->SetWidgetSpace(EWidgetSpace::Screen);
}

void ADTPBuildingActor::BeginPlay()
{
	Super::BeginPlay();

	// 建筑ID默认用Actor名称
	if (BuildingConfig && BuildingConfig->BuildingId.IsEmpty())
	{
		UE_LOG(LogDTP, Warning, TEXT("[DTP] 建筑 %s 未配置BuildingId"), *GetName());
	}
}

void ADTPBuildingActor::SetHighlighted(bool bHighlighted)
{
	bIsHighlighted = bHighlighted;

	// 外轮廓可见性
	if (OutlineMesh)
	{
		OutlineMesh->SetVisibility(bHighlighted);
	}

	// 信息悬浮Widget
	if (InfoWidget)
	{
		InfoWidget->SetVisibility(bHighlighted);
	}

	UpdateHighlightMaterial();
}

FDTPBuildingData ADTPBuildingActor::GetBuildingData() const
{
	FDTPBuildingData Data;
	if (BuildingConfig)
	{
		Data.BuildingId = BuildingConfig->BuildingId;
		Data.BuildingName = BuildingConfig->BuildingName;
		Data.FloorCount = BuildingConfig->FloorCount;
		Data.Area = BuildingConfig->Area;
	}

	// 填充实时数据
	Data.Occupancy = CurrentOccupancy;
	Data.PowerUsage = CurrentPowerUsage;
	Data.WaterUsage = CurrentWaterUsage;

	return Data;
}

void ADTPBuildingActor::UpdateData(const FDTPBuildingData& Data)
{
	CurrentOccupancy = Data.Occupancy;
	CurrentPowerUsage = Data.PowerUsage;
	CurrentWaterUsage = Data.WaterUsage;
}

void ADTPBuildingActor::UpdateHighlightMaterial()
{
	// 子类或蓝图可重载此方法实现自定义高亮效果
	// 默认实现：修改OutlineMesh的材质
}