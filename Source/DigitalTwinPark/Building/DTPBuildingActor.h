// DigitalTwinPark - 智慧园区数字孪生
// 建筑Actor - 场景中可选中、高亮、展示信息的建筑

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DTPBuildingActor.generated.h"

class UDTPBuildingDataAsset;
class UDTPBuildingLabelWidget;
struct FDTPBuildingData;

/**
 * 建筑Actor
 * 每栋建筑在场景中放置一个实例，绑定DataAsset配置
 * 支持点击选中、外轮廓高亮、信息弹窗
 */
UCLASS()
class DIGITALTWINPARK_API ADTPBuildingActor : public AActor
{
	GENERATED_BODY()

public:
	ADTPBuildingActor();

	// ========================================================================
	// 配置
	// ========================================================================

	/** 建筑配置DataAsset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Building")
	TObjectPtr<UDTPBuildingDataAsset> BuildingConfig;

	// ========================================================================
	// 高亮/选中
	// ========================================================================

	/** 设置高亮状态 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Building")
	void SetHighlighted(bool bHighlighted);

	// ========================================================================
	// 热力可视化（按实时数据给外轮廓变色）
	// ========================================================================

	/** 设置热力颜色（改外轮廓材质 Color 参数，绿→黄→红） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Building")
	void SetHeatColor(FLinearColor Color);

	/** 开/关热力模式（开启时外轮廓常显） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Building")
	void SetHeatMode(bool bEnabled);

	/** 是否处于热力模式 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|Building")
	bool IsHeatMode() const { return bHeatMode; }

	/** 选中高亮颜色（非热力模式下选中建筑时外轮廓的颜色） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Building|Highlight")
	FLinearColor SelectionColor = FLinearColor(0.10f, 0.80f, 1.00f, 1.0f);

	// ========================================================================
	// 数据浮动标签（建筑头顶常显）
	// ========================================================================

	/** 数据标签 Widget 类（留空则不显示浮动标签） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Building|Label")
	TSubclassOf<UDTPBuildingLabelWidget> LabelWidgetClass;

	/** 是否常显数据标签（关闭则恢复为选中时才显示信息组件） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Building|Label")
	bool bShowDataLabel = false;

	/** 标签距建筑原点的额外高度（微调悬浮位置） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Building|Label")
	float LabelHeightOffset = 0.0f;

	/** 标签渲染区域尺寸（像素）：Border 会填满该区域，按文字大小调整 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Building|Label")
	FVector2D LabelDrawSize = FVector2D(240.0f, 40.0f);

	/** 是否高亮 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|Building")
	bool IsHighlighted() const { return bIsHighlighted; }

	// ========================================================================
	// 数据
	// ========================================================================

	/** 获取建筑配置数据 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|Building")
	FDTPBuildingData GetBuildingData() const;

	/** 用实时IoT数据更新（由DataSubsystem驱动） */
	void UpdateData(const FDTPBuildingData& Data);

	/** 获取当前楼层人数 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|Building")
	int32 GetCurrentOccupancy() const { return CurrentOccupancy; }

	/** 获取当前功耗 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|Building")
	float GetCurrentPowerUsage() const { return CurrentPowerUsage; }

	// ========================================================================
	// 组件
	// ========================================================================

	/** 场景根组件（碰撞在此，用于点击选中） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Components")
	TObjectPtr<USceneComponent> RootScene;

	/** 碰撞盒（用于点击选中，包围整个建筑） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Components")
	TObjectPtr<class UBoxComponent> CollisionBox;

	/** 建筑网格（可选，蓝图子类可替换为ChildActorComponent） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Components")
	TObjectPtr<UStaticMeshComponent> BuildingMesh;

	/** 外轮廓高亮组件（运行时动态创建） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Components")
	TObjectPtr<UStaticMeshComponent> OutlineMesh;

	/** 信息悬浮Widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Components")
	TObjectPtr<class UWidgetComponent> InfoWidget;

protected:
	virtual void BeginPlay() override;

	/** 更新高亮材质 */
	void UpdateHighlightMaterial();

	/** 把颜色写到外轮廓材质参数 */
	void ApplyOutlineColor(const FLinearColor& Color);

	/** 创建/刷新数据浮动标签 */
	void UpdateLabelWidget();

	/** 数据浮动标签实例 */
	UPROPERTY()
	TObjectPtr<UDTPBuildingLabelWidget> LabelWidget;

	// 高亮材质
	UPROPERTY()
	TObjectPtr<UMaterialInterface> HighlightMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> OriginalMaterial;

	bool bIsHighlighted = false;

	// 热力模式
	bool bHeatMode = false;
	FLinearColor CurrentHeatColor = FLinearColor::White;

	// 实时数据
	int32 CurrentOccupancy = 0;
	float CurrentPowerUsage = 0.0f;
	float CurrentWaterUsage = 0.0f;
};