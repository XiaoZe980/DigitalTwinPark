// DigitalTwinPark - 智慧园区数字孪生
// 输入处理器 - 多平台输入适配

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DTPInputHandler.generated.h"

/**
 * 输入处理器
 * 放置在主场景中，负责多平台输入适配
 * 桌面：鼠标+键盘 → Enhanced Input
 * 大屏：触控手势 → 自定义手势识别
 * 移动：触控 → 陀螺仪+触控
 * Web：Pixel Streaming → 透传
 */
UCLASS()
class DIGITALTWINPARK_API ADTPInputHandler : public AActor
{
	GENERATED_BODY()

public:
	ADTPInputHandler();

	/** 是否在大屏触控模式 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|Input")
	bool IsTouchMode() const { return bIsTouchMode; }

	/** 切换到触控模式 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Input")
	void SetTouchMode(bool bTouchMode);

	/** 是否在移动端 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|Input")
	bool IsMobilePlatform() const;

protected:
	virtual void BeginPlay() override;

private:
	/** 检测当前平台 */
	void DetectPlatform();

	bool bIsTouchMode = false;
};