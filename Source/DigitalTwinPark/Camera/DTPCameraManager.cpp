// DigitalTwinPark - 智慧园区数字孪生
// 相机管理器实现

#include "DTPCameraManager.h"
#include "DTPCameraPreset.h"
#include "Building/DTPBuildingActor.h"
#include "DigitalTwinPark.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"

UDTPCameraManager::UDTPCameraManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDTPCameraManager::BeginPlay()
{
	Super::BeginPlay();

	// 缓存相机组件引用
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		CachedSpringArm = OwnerPawn->FindComponentByClass<USpringArmComponent>();
		CachedCamera = OwnerPawn->FindComponentByClass<UCameraComponent>();
	}
}

void UDTPCameraManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsFlying)
	{
		UpdateFlyTo(DeltaTime);
	}
}

void UDTPCameraManager::SetFreeRoamMode()
{
	CurrentMode = EDTPCameraMode::FreeRoam;
	bIsFlying = false;
	UE_LOG(LogDTP, Log, TEXT("[DTP] 相机模式: 自由漫游"));
}

void UDTPCameraManager::FlyToPreset(UDTPCameraPreset* Preset)
{
	if (!Preset || !CachedSpringArm) return;

	CurrentMode = EDTPCameraMode::Preset;
	FlyStartLocation = CachedSpringArm->GetComponentLocation();
	FlyStartRotation = CachedSpringArm->GetComponentRotation();
	FlyTargetLocation = Preset->Location;
	FlyTargetRotation = Preset->Rotation;
	FlyDuration = Preset->TransitionTime;
	FlyElapsed = 0.0f;
	bIsFlying = true;

	UE_LOG(LogDTP, Log, TEXT("[DTP] 飞行到预设: %s"), *Preset->PresetName.ToString());
}

void UDTPCameraManager::FocusOnBuilding(ADTPBuildingActor* Building)
{
	if (!Building || !CachedSpringArm) return;

	CurrentMode = EDTPCameraMode::FocusBuilding;

	FVector BuildingLocation = Building->GetActorLocation();
	FVector TargetLocation = BuildingLocation + FVector(-2000.0f, 0.0f, 1500.0f);
	FRotator TargetRotation = (BuildingLocation - TargetLocation).Rotation();

	FlyStartLocation = CachedSpringArm->GetComponentLocation();
	FlyStartRotation = CachedSpringArm->GetComponentRotation();
	FlyTargetLocation = TargetLocation;
	FlyTargetRotation = TargetRotation;
	FlyDuration = 0.8f;
	FlyElapsed = 0.0f;
	bIsFlying = true;

	UE_LOG(LogDTP, Log, TEXT("[DTP] 聚焦建筑: %s"), *Building->GetName());
}

void UDTPCameraManager::SetTopDownMode()
{
	CurrentMode = EDTPCameraMode::TopDown;

	if (CachedSpringArm)
	{
		FlyStartLocation = CachedSpringArm->GetComponentLocation();
		FlyStartRotation = CachedSpringArm->GetComponentRotation();
		FlyTargetLocation = FVector(0.0f, 0.0f, 15000.0f);
		FlyTargetRotation = FRotator(-90.0f, 0.0f, 0.0f);
		FlyDuration = 1.5f;
		FlyElapsed = 0.0f;
		bIsFlying = true;
	}

	UE_LOG(LogDTP, Log, TEXT("[DTP] 相机模式: 鸟瞰"));
}

void UDTPCameraManager::UpdateFlyTo(float DeltaTime)
{
	FlyElapsed += DeltaTime;
	float Alpha = FMath::Clamp(FlyElapsed / FlyDuration, 0.0f, 1.0f);

	// 使用SmoothStep插值，起始和结尾更平滑
	float T = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);

	if (CachedSpringArm)
	{
		// 直接设置SpringArm的世界位置和旋转
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (OwnerPawn)
		{
			FVector NewLocation = FMath::Lerp(FlyStartLocation, FlyTargetLocation, T);
			OwnerPawn->SetActorLocation(NewLocation);

			// 旋转通过Controller
			if (AController* Controller = OwnerPawn->GetController())
			{
				FRotator NewRotation = FMath::Lerp(FlyStartRotation, FlyTargetRotation, T);
				Controller->SetControlRotation(NewRotation);
			}
		}
	}

	if (Alpha >= 1.0f)
	{
		bIsFlying = false;
	}
}