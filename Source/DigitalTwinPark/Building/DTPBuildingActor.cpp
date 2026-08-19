// DigitalTwinPark - 智慧园区数字孪生
// 建筑Actor实现

#include "DTPBuildingActor.h"
#include "DTPBuildingDataAsset.h"
#include "Data/DTPDataTypes.h"
#include "DigitalTwinPark.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Materials/MaterialInterface.h"

ADTPBuildingActor::ADTPBuildingActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 根组件
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootScene);

	// 碰撞盒（用于点击选中，编辑器可见，游戏中隐藏）
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootScene);
	CollisionBox->SetBoxExtent(FVector(500.0f, 500.0f, 500.0f));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetHiddenInGame(true);

	// 外轮廓（默认隐藏）
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