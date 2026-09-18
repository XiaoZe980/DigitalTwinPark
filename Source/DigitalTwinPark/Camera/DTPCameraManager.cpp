// DigitalTwinPark - 智慧园区数字孪生
// 相机管理器实现

#include "DTPCameraManager.h"
#include "DTPCameraPreset.h"
#include "Building/DTPBuildingActor.h"
#include "Core/DTPPawn.h"
#include "DigitalTwinPark.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "UObject/Package.h"

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

	// Presets 未配置时按默认路径加载
	// （运行时 Pawn 由 GameMode 动态生成，拿不到关卡里编辑用 Pawn 上配的数组）
	if (Presets.Num() == 0)
	{
		static const TCHAR* DefaultPresetPaths[] = {
			TEXT("/Game/Data/CP_Overview.CP_Overview"),
			TEXT("/Game/Data/CP_TopDown.CP_TopDown"),
			TEXT("/Game/Data/CP_Entrance.CP_Entrance"),
			TEXT("/Game/Data/CP_Building_A.CP_Building_A"),
		};

		for (const TCHAR* Path : DefaultPresetPaths)
		{
			if (UDTPCameraPreset* Preset = LoadObject<UDTPCameraPreset>(nullptr, Path))
			{
				Presets.Add(Preset);
			}
			else
			{
				UE_LOG(LogDTP, Warning, TEXT("[DTP] 相机预设加载失败: %s"), Path);
			}
		}

		UE_LOG(LogDTP, Log, TEXT("[DTP] CameraManager 自动加载 %d 个相机预设"), Presets.Num());
	}
}

void UDTPCameraManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsFlying)
	{
		UpdateFlyTo(DeltaTime);
	}

	// 自动巡游：飞到预设 → 停留 → 飞下一个
	if (bAutoTour && !bIsFlying && Presets.Num() > 0)
	{
		if (!bTourWaiting)
		{
			// 刚到达，开始计时停留
			bTourWaiting = true;
			TourWaitTimer = 0.0f;
		}
		else
		{
			TourWaitTimer += DeltaTime;
			if (TourWaitTimer >= TourWaitTime)
			{
				bTourWaiting = false;
				TourWaitTimer = 0.0f;

				if (Presets.IsValidIndex(TourPresetIndex) && Presets[TourPresetIndex])
				{
					FlyToPreset(Presets[TourPresetIndex]);
				}
				TourPresetIndex = (TourPresetIndex + 1) % Presets.Num();
			}
		}
	}
}

void UDTPCameraManager::StartAutoTour(float Interval)
{
	if (Presets.Num() == 0)
	{
		UE_LOG(LogDTP, Warning, TEXT("[DTP] StartAutoTour: Presets 为空，无法巡游"));
		return;
	}

	bAutoTour = true;
	TourWaitTime = FMath::Max(1.0f, Interval);
	TourWaitTimer = 0.0f;
	bTourWaiting = false;
	TourPresetIndex = 0;

	// 立即飞向第一个预设
	if (Presets[0])
	{
		FlyToPreset(Presets[0]);
	}
	TourPresetIndex = 1 % Presets.Num();

	UE_LOG(LogDTP, Log, TEXT("[DTP] 自动巡游开始（停留 %.1f 秒/视角，共 %d 个视角）"), TourWaitTime, Presets.Num());
}

void UDTPCameraManager::StopAutoTour()
{
	if (!bAutoTour) return;

	bAutoTour = false;
	bTourWaiting = false;
	TourWaitTimer = 0.0f;

	UE_LOG(LogDTP, Log, TEXT("[DTP] 自动巡游结束"));
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
	// 起点取当前 Pawn 位置 + 控制旋转（与 UpdateFlyTo 实际操作量一致），保证从当前角度平滑过渡
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		FlyStartLocation = OwnerPawn->GetActorLocation();
		FlyStartRotation = OwnerPawn->GetControlRotation();
	}
	else
	{
		FlyStartLocation = CachedSpringArm->GetComponentLocation();
		FlyStartRotation = CachedSpringArm->GetComponentRotation();
	}
	FlyTargetLocation = Preset->Location;
	FlyTargetRotation = Preset->Rotation;
	// 过渡时间兜底：TransitionTime 未配置(0)时给 0.5s，避免瞬间跳变
	FlyDuration = FMath::Max(Preset->TransitionTime, 0.5f);
	FlyElapsed = 0.0f;
	bIsFlying = true;

	UE_LOG(LogDTP, Log, TEXT("[DTP] 飞行到预设: %s (%.2fs) 起点=(%.0f,%.0f,%.0f) 终点=(%.0f,%.0f,%.0f)"),
		*Preset->PresetName.ToString(), FlyDuration,
		FlyStartLocation.X, FlyStartLocation.Y, FlyStartLocation.Z,
		FlyTargetLocation.X, FlyTargetLocation.Y, FlyTargetLocation.Z);
}

void UDTPCameraManager::FocusOnBuilding(ADTPBuildingActor* Building)
{
	if (!Building || !CachedSpringArm) return;

	CurrentMode = EDTPCameraMode::FocusBuilding;

	// 起点：当前 Pawn 位置 + 控制旋转（与飞行实际操作量一致），并收短臂长
	FVector StartLocation;
	if (ADTPPawn* OwnerPawn = Cast<ADTPPawn>(GetOwner()))
	{
		StartLocation = OwnerPawn->GetActorLocation();
		FlyStartRotation = OwnerPawn->GetControlRotation();
		OwnerPawn->SetDesiredArmLength(0.0f); // 钳到最小臂长，让相机贴近聚焦点
	}
	else
	{
		StartLocation = CachedSpringArm->GetComponentLocation();
		FlyStartRotation = CachedSpringArm->GetComponentRotation();
	}
	FlyStartLocation = StartLocation;

	// 建筑包围盒 → 自适应距离（大建筑拉远、小建筑拉近）
	FVector Origin, Extent;
	Building->GetActorBounds(true, Origin, Extent);
	const float Radius = FMath::Max3(Extent.X, Extent.Y, Extent.Z);
	const float Distance = FMath::Max(Radius * 2.2f, 1200.0f);

	// 保持当前方位：从相机所在方向飞近建筑，而不是固定绕到某一侧
	FVector HorizDir = StartLocation - Origin;
	HorizDir.Z = 0.0f;
	if (HorizDir.IsNearlyZero())
	{
		HorizDir = FVector(-1.0f, 0.0f, 0.0f); // 相机正好在建筑正上方时的兜底
	}
	HorizDir.Normalize();

	// 当前水平方位 + 仰角，从斜上方俯视建筑
	const FVector ViewDir = (HorizDir + FVector(0.0f, 0.0f, 0.6f)).GetSafeNormal();
	FlyTargetLocation = Origin + ViewDir * Distance;
	FlyTargetRotation = (Origin - FlyTargetLocation).Rotation();

	FlyDuration = 1.2f;
	FlyElapsed = 0.0f;
	bIsFlying = true;

	UE_LOG(LogDTP, Log, TEXT("[DTP] 聚焦建筑: %s (距离 %.0f)"), *Building->GetName(), Distance);
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

			// 旋转通过Controller，四元数Slerp走最短旋转路径，避免绕大圈
			if (AController* Controller = OwnerPawn->GetController())
			{
				const FQuat NewQuat = FQuat::Slerp(
					FlyStartRotation.Quaternion(),
					FlyTargetRotation.Quaternion(),
					T);
				Controller->SetControlRotation(NewQuat.Rotator());
			}
		}
	}

	if (Alpha >= 1.0f)
	{
		bIsFlying = false;
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		UE_LOG(LogDTP, Log, TEXT("[DTP] 飞行到预设完成, Pawn落点=(%.0f,%.0f,%.0f)"),
			OwnerPawn ? OwnerPawn->GetActorLocation().X : 0.0f,
			OwnerPawn ? OwnerPawn->GetActorLocation().Y : 0.0f,
			OwnerPawn ? OwnerPawn->GetActorLocation().Z : 0.0f);
	}
}

void UDTPCameraManager::SaveCurrentViewToPreset()
{
	// 编辑器工具（CallInEditor）：把当前 Pawn 位置/旋转写进 Presets[PresetIndexToSave] 并落盘
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		UE_LOG(LogDTP, Warning, TEXT("[DTP] 保存预设失败：未找到 Pawn"));
		return;
	}
	if (!Presets.IsValidIndex(PresetIndexToSave) || !Presets[PresetIndexToSave])
	{
		UE_LOG(LogDTP, Warning, TEXT("[DTP] 保存预设失败：Presets[%d] 未配置"), PresetIndexToSave);
		return;
	}

	UDTPCameraPreset* Preset = Presets[PresetIndexToSave];
	Preset->Location = OwnerPawn->GetActorLocation();
	Preset->Rotation = OwnerPawn->GetActorRotation();
	Preset->MarkPackageDirty();

	UE_LOG(LogDTP, Log, TEXT("[DTP] 已保存预设[%d] %s → 位置=(%.0f,%.0f,%.0f) 旋转=(%.1f,%.1f,%.1f)"),
		PresetIndexToSave, *Preset->PresetName.ToString(),
		Preset->Location.X, Preset->Location.Y, Preset->Location.Z,
		Preset->Rotation.Pitch, Preset->Rotation.Yaw, Preset->Rotation.Roll);
}