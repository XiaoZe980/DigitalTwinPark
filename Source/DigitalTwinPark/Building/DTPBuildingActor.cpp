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
#include "UI/DTPBuildingLabelWidget.h"
#include "Blueprint/UserWidget.h"

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

	// 建筑网格（默认创建，蓝图里可直接指定静态网格）
	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	BuildingMesh->SetupAttachment(RootScene);
	BuildingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

	// 数据浮动标签：常显在建筑头顶
	if (bShowDataLabel && LabelWidgetClass && InfoWidget)
	{
		LabelWidget = CreateWidget<UDTPBuildingLabelWidget>(GetWorld(), LabelWidgetClass);
		if (LabelWidget)
		{
			InfoWidget->SetWidget(LabelWidget);
			InfoWidget->SetDrawSize(LabelDrawSize);
			InfoWidget->SetRelativeLocation(FVector(0.0f, 0.0f, LabelHeightOffset));
			InfoWidget->SetVisibility(true);
			UpdateLabelWidget();
		}
	}
}

void ADTPBuildingActor::SetHighlighted(bool bHighlighted)
{
	bIsHighlighted = bHighlighted;

	// 外轮廓可见性（选中 或 热力模式 时显示）
	if (OutlineMesh)
	{
		OutlineMesh->SetVisibility(bHighlighted || bHeatMode);

		// 非热力模式用固定的选中高亮色，避免残留上一次的热力颜色
		if (!bHeatMode)
		{
			ApplyOutlineColor(SelectionColor);
		}
	}

	// 信息悬浮Widget（标签模式常显，否则选中时才显示）
	if (InfoWidget && !bShowDataLabel)
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

	// 同步刷新头顶标签
	UpdateLabelWidget();
}

void ADTPBuildingActor::UpdateLabelWidget()
{
	if (LabelWidget)
	{
		LabelWidget->UpdateLabelData(GetBuildingData());
	}
}

void ADTPBuildingActor::UpdateHighlightMaterial()
{
	// 子类或蓝图可重载此方法实现自定义高亮效果
	// 默认实现：修改OutlineMesh的材质
}

void ADTPBuildingActor::ApplyOutlineColor(const FLinearColor& Color)
{
	if (OutlineMesh)
	{
		// 改外轮廓材质 Color 参数（材质需有名为 Color 的 Vector 参数）
		OutlineMesh->SetVectorParameterValueOnMaterials(
			TEXT("Color"), FVector(Color.R, Color.G, Color.B));
	}
}

void ADTPBuildingActor::SetHeatColor(FLinearColor Color)
{
	CurrentHeatColor = Color;
	// 仅热力模式下才覆盖外轮廓颜色，避免影响选中高亮
	if (bHeatMode)
	{
		ApplyOutlineColor(Color);
	}
}

void ADTPBuildingActor::SetHeatMode(bool bEnabled)
{
	bHeatMode = bEnabled;
	if (OutlineMesh)
	{
		OutlineMesh->SetVisibility(bEnabled || bIsHighlighted);
		// 开启热力用当前热力色；关闭则恢复选中高亮色
		ApplyOutlineColor(bEnabled ? CurrentHeatColor : SelectionColor);
	}
}