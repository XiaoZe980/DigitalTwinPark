// DigitalTwinPark - 智慧园区数字孪生
// 交通模拟器 - Spline路径车流

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DTPTrafficManager.generated.h"

class USplineComponent;
class USkeletalMesh;
class USkeletalMeshComponent;

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

	/** 蓝图车辆类（如 BP_MCar01~03，可多选，生成时随机挑选） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Traffic")
	TArray<TSubclassOf<AActor>> VehicleClasses;

	/** 静态网格车（可多选，随机挑选） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Traffic")
	TArray<TObjectPtr<UStaticMesh>> VehicleMeshes;

	/** 骨骼网格车（可多选，随机挑选；无动画时为默认骨骼姿态） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Traffic")
	TArray<TObjectPtr<USkeletalMesh>> SkeletalMeshes;

	/** 车辆间距 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Traffic")
	float VehicleSpacing = 500.0f;

	/** 车辆数量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Traffic", meta = (DisplayName = "Vehicle Count"))
	int32 TargetVehicleCount = 20;

	/** 平均速度 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Traffic", meta = (DisplayName = "Average Speed"))
	float TargetAverageSpeed = 40.0f;

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
		TObjectPtr<UStaticMeshComponent> Mesh;          // 静态网格模式
		TObjectPtr<USkeletalMeshComponent> SkeletalMesh; // 骨骼网格模式
		TObjectPtr<AActor> VehicleActor;                // 蓝图车辆模式
		float SplineDistance = 0.0f;
		float Speed = 0.0f;
	};

	TArray<FVehicleInstance> Vehicles;
};