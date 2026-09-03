// DigitalTwinPark - 智慧园区数字孪生
// 观察者Pawn实现

#include "DTPPawn.h"
#include "DigitalTwinPark.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerController.h"
#include "Camera/DTPCameraManager.h"

ADTPPawn::ADTPPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// 根组件
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComp);

	// 浮空移动组件
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	MovementComponent->SetPlaneConstraintEnabled(false); // 无平面约束，自由飞行
	MovementComponent->MaxSpeed = MoveSpeed; // 实际速度由 MaxSpeed 控制（AddMovementInput 的幅度会被夹紧到 1）

	// 弹簧臂（相机悬臂）
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 5000.0f;
	DesiredArmLength = SpringArm->TargetArmLength; // 目标臂长与初始一致
	// 关键：弹簧臂跟随控制旋转，右键拖动旋转才能驱动相机
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bDoCollisionTest = false; // 不碰撞检测，避免穿模卡顿
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 5.0f;

	// 相机
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// 相机管理器（预设视角飞行等，Editor 里在组件详情页配 Presets）
	CameraManager = CreateDefaultSubobject<UDTPCameraManager>(TEXT("CameraManager"));
}

void ADTPPawn::BeginPlay()
{
	Super::BeginPlay();

	// 同步蓝图里可能修改过的 MoveSpeed
	MovementComponent->MaxSpeed = MoveSpeed;
	UE_LOG(LogDTP, Log, TEXT("[DTP] Pawn BeginPlay, MaxSpeed=%.1f"), MovementComponent->MaxSpeed);
}

void ADTPPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 缩放：目标臂长线性插值逼近，滚轮不跳变
	SpringArm->TargetArmLength = FMath::FInterpTo(
		SpringArm->TargetArmLength,
		DesiredArmLength,
		DeltaTime,
		ZoomInterpSpeed
	);
}

void ADTPPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 初始俯视 45°，覆盖 GameMode 默认的 (0,0,0)
	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		PC->SetControlRotation(FRotator(-45.0f, 0.0f, 0.0f));
	}
}

void ADTPPawn::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// 只取 Yaw 方向：保证 W/S 水平前后移动（不随俯仰角钻地/上漂），高度用 Q/E 控制
		const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ADTPPawn::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// 只取 Yaw 方向，水平左右
		const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ADTPPawn::MoveUp(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(FVector::UpVector, Value);
	}
}

void ADTPPawn::LookAt(float DeltaX, float DeltaY)
{
	// 手动管理控制旋转：水平（Yaw）+ 垂直（Pitch，夹紧范围防翻转）
	// 拖拽式：鼠标下滑 → Pitch 增大（视角上抬），场景跟手
	FRotator NewRotation = GetControlRotation();
	NewRotation.Yaw += DeltaX * LookSensitivity;
	NewRotation.Pitch = FMath::Clamp(
		NewRotation.Pitch + DeltaY * LookSensitivity,
		MinLookPitch,
		MaxLookPitch
	);
	if (AController* MyController = GetController())
	{
		MyController->SetControlRotation(NewRotation);
	}
}

void ADTPPawn::Zoom(float Amount)
{
	// 只更新目标臂长，实际由 Tick 线性插值过渡
	DesiredArmLength = FMath::Clamp(
		DesiredArmLength - Amount * ZoomSpeed,
		MinZoomDistance,
		MaxZoomDistance
	);
}