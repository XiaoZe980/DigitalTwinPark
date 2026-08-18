// DigitalTwinPark - 智慧园区数字孪生
// 交通模拟器 - Spline路径车流

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DTPTrafficManager.generated.h"

class USplineComponent;

/**
 * 交通模拟器
 * 在Spline路径上生成循环移动的车辆，模拟园区道路车流
 */
UCLASS()
class DIGITALTWINPARK_API ADTPTrafficManager : public AActor
{
	GENERATED_BODY()

public:
	ADTPTrafficManager();

	/** 设置车辆数量 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Traffic")
	void SetVehicleCount(int32 Count);

	/** 设置平均速度 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Traffic")
	void SetAverageSpeed(float Speed);

	// ========================================================================
	// 配置
	// ========================================================================

	/** 道路Spline */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Components")
	TObjectPtr<USplineComponent> RoadSpline;

	/** 车辆Mesh（可指定多种随机选择） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Traffic")
	TArray<TObjectPtr<UStaticMesh>> VehicleMeshes;

	/** 车辆间距 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Traffic")
	float VehicleSpacing = 500.0f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	/** 生成车辆 */
	void SpawnVehicles();

	/** 更新车辆位置 */
	void UpdateVehiclePositions(float DeltaTime);

	/** 单辆车的数据 */
	struct FVehicleInstance
	{
		TObjectPtr<UStaticMeshComponent> Mesh;
		float SplineDistance = 0.0f;
		float Speed = 0.0f;
	};

	TArray<FVehicleInstance> Vehicles;
	int32 TargetVehicleCount = 20;
	float TargetAverageSpeed = 40.0f;
};