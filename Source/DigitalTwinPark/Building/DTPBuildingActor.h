// DigitalTwinPark - 智慧园区数字孪生
// 建筑Actor - 场景中可选中、高亮、展示信息的建筑

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DTPBuildingActor.generated.h"

class UDTPBuildingDataAsset;
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

	// 高亮材质
	UPROPERTY()
	TObjectPtr<UMaterialInterface> HighlightMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> OriginalMaterial;

	bool bIsHighlighted = false;

	// 实时数据
	int32 CurrentOccupancy = 0;
	float CurrentPowerUsage = 0.0f;
	float CurrentWaterUsage = 0.0f;
};