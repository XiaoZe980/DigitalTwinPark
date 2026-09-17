// DigitalTwinPark - 智慧园区数字孪生
// 建筑管理器 - 注册、查找场景中的建筑

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DTPBuildingManager.generated.h"

class ADTPBuildingActor;

/**
 * 建筑管理器
 * 放置在场景中，自动收集所有ADTPBuildingActor
 * 提供建筑查找、数据同步等功能
 */
UCLASS()
class DIGITALTWINPARK_API ADTPBuildingManager : public AActor
{
	GENERATED_BODY()

public:
	ADTPBuildingManager();

	/** 获取所有注册的建筑 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Building")
	const TArray<ADTPBuildingActor*>& GetAllBuildings() const { return RegisteredBuildings; }

	/** 根据ID查找建筑 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Building")
	ADTPBuildingActor* FindBuildingById(const FString& BuildingId) const;

	/** 手动注册建筑 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Building")
	void RegisterBuilding(ADTPBuildingActor* Building);

	/** 刷新建筑注册（重新扫描场景） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Building")
	void RefreshBuildings();

	// ========================================================================
	// 热力变色（按实时能耗给建筑外轮廓上色）
	// ========================================================================

	/** 开/关建筑热力变色（绿=低负载 → 黄=中 → 红=高负载） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Building")
	void SetHeatMapEnabled(bool bEnabled);

	/** 是否开启热力变色 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|Building")
	bool IsHeatMapEnabled() const { return bHeatMapEnabled; }

	/** 热力映射上限（能耗 kW，达到此值显示红色） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Building|HeatMap")
	float HeatMapMaxPower = 1000.0f;

protected:
	virtual void BeginPlay() override;

	/** 数据更新回调：把 DataSubsystem 最新数据同步到所有建筑 */
	UFUNCTION()
	void HandleBuildingDataUpdated();

	/** 按当前能耗刷新所有建筑的热力颜色 */
	void UpdateHeatMap();

	UPROPERTY()
	TArray<ADTPBuildingActor*> RegisteredBuildings;

	/** 热力变色是否开启 */
	bool bHeatMapEnabled = false;
};