// DigitalTwinPark - 智慧园区数字孪生
// 交通模拟器实现

#include "DTPTrafficManager.h"
#include "DigitalTwinPark.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

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
		if (Vehicle.SkeletalMesh)
		{
			Vehicle.SkeletalMesh->DestroyComponent();
		}
		if (Vehicle.VehicleActor)
		{
			Vehicle.VehicleActor->Destroy();
		}
	}
	Vehicles.Empty();

	// 三类车辆候选池：蓝图 / 静态网格 / 骨骼网格，混合随机
	const int32 TotalCandidates = VehicleClasses.Num() + VehicleMeshes.Num() + SkeletalMeshes.Num();
	if (!RoadSpline || TotalCandidates == 0) return;

	const float SplineLength = RoadSpline->GetSplineLength();
	const float Spacing = SplineLength / FMath::Max(1, TargetVehicleCount);

	for (int32 i = 0; i < TargetVehicleCount; ++i)
	{
		FVehicleInstance Vehicle;
		Vehicle.SplineDistance = i * Spacing + FMath::FRandRange(0.0f, Spacing * 0.5f);
		// 每辆车独立速度：40%~160% 随机，拉开快慢差异（避免车流整齐划一）
		Vehicle.Speed = TargetAverageSpeed * FMath::FRandRange(0.4f, 1.6f);

		const FVector Location = RoadSpline->GetLocationAtDistanceAlongSpline(Vehicle.SplineDistance, ESplineCoordinateSpace::World);
		const FRotator Rotation = RoadSpline->GetRotationAtDistanceAlongSpline(Vehicle.SplineDistance, ESplineCoordinateSpace::World);

		const int32 Pick = FMath::RandRange(0, TotalCandidates - 1);
		if (Pick < VehicleClasses.Num())
		{
			// 蓝图车辆：生成完整车辆 Actor
			Vehicle.VehicleActor = GetWorld()->SpawnActor<AActor>(VehicleClasses[Pick], Location, Rotation);
		}
		else if (Pick < VehicleClasses.Num() + VehicleMeshes.Num())
		{
			// 静态网格车
			const int32 MeshIdx = Pick - VehicleClasses.Num();
			Vehicle.Mesh = NewObject<UStaticMeshComponent>(this);
			Vehicle.Mesh->RegisterComponent();
			Vehicle.Mesh->AttachToComponent(RoadSpline, FAttachmentTransformRules::KeepRelativeTransform);
			Vehicle.Mesh->SetStaticMesh(VehicleMeshes[MeshIdx]);
			Vehicle.Mesh->SetWorldLocation(Location);
			Vehicle.Mesh->SetWorldRotation(Rotation);
		}
		else
		{
			// 骨骼网格车
			const int32 SkelIdx = Pick - VehicleClasses.Num() - VehicleMeshes.Num();
			Vehicle.SkeletalMesh = NewObject<USkeletalMeshComponent>(this);
			Vehicle.SkeletalMesh->RegisterComponent();
			Vehicle.SkeletalMesh->AttachToComponent(RoadSpline, FAttachmentTransformRules::KeepRelativeTransform);
			Vehicle.SkeletalMesh->SetSkeletalMesh(SkeletalMeshes[SkelIdx]);
			Vehicle.SkeletalMesh->SetWorldLocation(Location);
			Vehicle.SkeletalMesh->SetWorldRotation(Rotation);
		}

		Vehicles.Add(Vehicle);
	}

	UE_LOG(LogDTP, Log, TEXT("[DTP] 交通模拟: 生成 %d 辆车（候选 %d 类）"), Vehicles.Num(), TotalCandidates);
}

void ADTPTrafficManager::UpdateVehiclePositions(float DeltaTime)
{
	if (!RoadSpline) return;

	const float SplineLength = RoadSpline->GetSplineLength();

	for (auto& Vehicle : Vehicles)
	{
		// 沿Spline移动
		Vehicle.SplineDistance += Vehicle.Speed * DeltaTime * 100.0f; // cm/s

		// 循环
		if (Vehicle.SplineDistance > SplineLength)
		{
			Vehicle.SplineDistance -= SplineLength;
		}

		// 更新位置和旋转
		const FVector Location = RoadSpline->GetLocationAtDistanceAlongSpline(Vehicle.SplineDistance, ESplineCoordinateSpace::World);
		const FRotator Rotation = RoadSpline->GetRotationAtDistanceAlongSpline(Vehicle.SplineDistance, ESplineCoordinateSpace::World);

		if (Vehicle.VehicleActor)
		{
			Vehicle.VehicleActor->SetActorLocation(Location);
			Vehicle.VehicleActor->SetActorRotation(Rotation);
		}
		else if (Vehicle.Mesh)
		{
			Vehicle.Mesh->SetWorldLocation(Location);
			Vehicle.Mesh->SetWorldRotation(Rotation);
		}
		else if (Vehicle.SkeletalMesh)
		{
			Vehicle.SkeletalMesh->SetWorldLocation(Location);
			Vehicle.SkeletalMesh->SetWorldRotation(Rotation);
		}
	}
}