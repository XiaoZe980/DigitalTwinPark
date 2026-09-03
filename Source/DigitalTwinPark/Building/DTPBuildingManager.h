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

protected:
	virtual void BeginPlay() override;

	/** 数据更新回调：把 DataSubsystem 最新数据同步到所有建筑 */
	UFUNCTION()
	void HandleBuildingDataUpdated();

	UPROPERTY()
	TArray<ADTPBuildingActor*> RegisteredBuildings;
};