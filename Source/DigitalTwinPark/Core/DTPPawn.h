// DigitalTwinPark - 智慧园区数字孪生
// 观察者Pawn - 自由飞行浏览场景

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DTPPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;

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

	/** 旋转灵敏度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Movement")
	float LookSensitivity = 1.0f;

	/** 缩放速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Movement")
	float ZoomSpeed = 500.0f;

	/** 缩放范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Movement")
	float MinZoomDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Movement")
	float MaxZoomDistance = 20000.0f;

	// ========================================================================
	// 组件
	// ========================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Components")
	TObjectPtr<UFloatingPawnMovement> MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Components")
	TObjectPtr<UCameraComponent> Camera;

protected:
	virtual void BeginPlay() override;
};