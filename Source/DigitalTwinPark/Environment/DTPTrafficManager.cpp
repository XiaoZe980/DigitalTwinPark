// DigitalTwinPark - 智慧园区数字孪生
// 交通模拟器实现

#include "DTPTrafficManager.h"
#include "DigitalTwinPark.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"

ADTPTrafficManager::ADTPTrafficManager()
{
	PrimaryActorTick.bCanEverTick = true;

	RoadSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RoadSpline"));
	SetRootComponent(RoadSpline);
}

void ADTPTrafficManager::BeginPlay()
{
	Super::BeginPlay();
	SpawnVehicles();
}

void ADTPTrafficManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateVehiclePositions(DeltaTime);
}

void ADTPTrafficManager::SetVehicleCount(int32 Count)
{
	TargetVehicleCount = FMath::Max(0, Count);
}

void ADTPTrafficManager::SetAverageSpeed(float Speed)
{
	TargetAverageSpeed = FMath::Max(0.0f, Speed);
}

void ADTPTrafficManager::SpawnVehicles()
{
	// 清理旧车辆
	for (auto& Vehicle : Vehicles)
	{
		if (Vehicle.Mesh)
		{
			Vehicle.Mesh->DestroyComponent();
		}
	}
	Vehicles.Empty();

	if (VehicleMeshes.IsEmpty() || !RoadSpline) return;

	const float SplineLength = RoadSpline->GetSplineLength();
	const float Spacing = SplineLength / FMath::Max(1, TargetVehicleCount);

	for (int32 i = 0; i < TargetVehicleCount; ++i)
	{
		FVehicleInstance Vehicle;
		Vehicle.SplineDistance = i * Spacing + FMath::FRandRange(0.0f, Spacing * 0.5f);
		Vehicle.Speed = TargetAverageSpeed * (0.8f + FMath::FRandRange(0.0f, 0.4f)); // 速度随机变化±20%

		// 创建车辆Mesh
		Vehicle.Mesh = NewObject<UStaticMeshComponent>(this);
		Vehicle.Mesh->RegisterComponent();
		Vehicle.Mesh->AttachToComponent(RoadSpline, FAttachmentTransformRules::KeepRelativeTransform);

		// 随机选择车辆模型
		const int32 MeshIdx = FMath::RandRange(0, VehicleMeshes.Num() - 1);
		Vehicle.Mesh->SetStaticMesh(VehicleMeshes[MeshIdx]);

		// 初始位置
		FVector Location = RoadSpline->GetLocationAtDistanceAlongSpline(Vehicle.SplineDistance, ESplineCoordinateSpace::World);
		Vehicle.Mesh->SetWorldLocation(Location);

		Vehicles.Add(Vehicle);
	}

	UE_LOG(LogDTP, Log, TEXT("[DTP] 交通模拟: 生成 %d 辆车"), Vehicles.Num());
}

void ADTPTrafficManager::UpdateVehiclePositions(float DeltaTime)
{
	if (!RoadSpline) return;

	const float SplineLength = RoadSpline->GetSplineLength();

	for (auto& Vehicle : Vehicles)
	{
		if (!Vehicle.Mesh) continue;

		// 沿Spline移动
		Vehicle.SplineDistance += Vehicle.Speed * DeltaTime * 100.0f; // cm/s

		// 循环
		if (Vehicle.SplineDistance > SplineLength)
		{
			Vehicle.SplineDistance -= SplineLength;
		}

		// 更新位置和旋转
		FVector Location = RoadSpline->GetLocationAtDistanceAlongSpline(Vehicle.SplineDistance, ESplineCoordinateSpace::World);
		FRotator Rotation = RoadSpline->GetRotationAtDistanceAlongSpline(Vehicle.SplineDistance, ESplineCoordinateSpace::World);

		Vehicle.Mesh->SetWorldLocation(Location);
		Vehicle.Mesh->SetWorldRotation(Rotation);
	}
}