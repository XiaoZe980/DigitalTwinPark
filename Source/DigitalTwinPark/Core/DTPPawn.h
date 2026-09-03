// DigitalTwinPark - 智慧园区数字孪生
// 观察者Pawn - 自由飞行浏览场景

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DTPPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;
class UDTPCameraManager;

/**
 * 数字孪生观察者Pawn
 * 无重力的自由飞行模式，支持鼠标/键盘和触控操作
 * 使用FloatingPawnMovement提供平滑移动
 */
UCLASS()
class DIGITALTWINPARK_API ADTPPawn : public APawn
{
	GENERATED_BODY()

public:
	ADTPPawn();

	// ========================================================================
	// 移动输入
	// ========================================================================

	/** 前后左右移动（X=前后, Y=左右） */
	void MoveForward(float Value);
	void MoveRight(float Value);

	/** 上下移动（Q/E或触控） */
	void MoveUp(float Value);

	/** 鼠标/触控视角旋转 */
	void LookAt(float DeltaX, float DeltaY);

	/** 缩放（滚轮或捏合手势） */
	void Zoom(float Amount);

	// ========================================================================
	// 配置
	// ========================================================================

	/** 移动速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Movement")
	float MoveSpeed = 2000.0f;

	/** 旋转灵敏度（鼠标像素增量需除以 ~3 才能手感自然） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Movement")
	float LookSensitivity = 0.4f;

	/** 俯仰角最小限制（负值低头看，防止视角翻转） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Movement")
	float MinLookPitch = -85.0f;

	/** 俯仰角最大限制（0 为水平，园区浏览不需要仰视） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Movement")
	float MaxLookPitch = 0.0f;

	/** 缩放速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Movement")
	float ZoomSpeed = 500.0f;

	/** 缩放范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Movement")
	float MinZoomDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Movement")
	float MaxZoomDistance = 20000.0f;

	/** 缩放插值速度（线性逼近目标臂长，越大越跟手） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Movement")
	float ZoomInterpSpeed = 10.0f;

	// ========================================================================
	// 组件
	// ========================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Components")
	TObjectPtr<UFloatingPawnMovement> MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Components")
	TObjectPtr<UCameraComponent> Camera;

	/** 相机管理器（预设视角飞行、鸟瞰切换等） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Components")
	TObjectPtr<UDTPCameraManager> CameraManager;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;

private:
	/** 缩放目标臂长（Tick 中线性插值逼近，避免滚轮跳变卡顿） */
	float DesiredArmLength = 5000.0f;
};