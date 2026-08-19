// DigitalTwinPark - 智慧园区数字孪生
// 观察者Pawn实现

#include "DTPPawn.h"
#include "DigitalTwinPark.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

ADTPPawn::ADTPPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// 根组件
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComp);

	// 浮空移动组件
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	MovementComponent->SetPlaneConstraintEnabled(false); // 无平面约束，自由飞行

	// 弹簧臂（相机悬臂）
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 5000.0f;
	SpringArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f)); // 俯视45度
	SpringArm->bDoCollisionTest = false; // 不碰撞检测，避免穿模卡顿
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 5.0f;

	// 相机
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

void ADTPPawn::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogDTP, Log, TEXT("[DTP] Pawn BeginPlay"));
}

void ADTPPawn::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X), Value * MoveSpeed);
	}
}

void ADTPPawn::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::Y), Value * MoveSpeed);
	}
}

void ADTPPawn::MoveUp(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(FVector::UpVector, Value * MoveSpeed);
	}
}

void ADTPPawn::LookAt(float DeltaX, float DeltaY)
{
	// 水平旋转
	AddControllerYawInput(DeltaX * LookSensitivity);
	// 垂直旋转
	AddControllerPitchInput(-DeltaY * LookSensitivity);
}

void ADTPPawn::Zoom(float Amount)
{
	SpringArm->TargetArmLength = FMath::Clamp(
		SpringArm->TargetArmLength - Amount * ZoomSpeed,
		MinZoomDistance,
		MaxZoomDistance
	);
}