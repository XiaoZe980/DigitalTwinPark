// DigitalTwinPark - 智慧园区数字孪生
// 昼夜循环 - 24小时光照模拟

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DTPDayNightCycle.generated.h"

class UDirectionalLightComponent;
class USkyLightComponent;
class USkyAtmosphereComponent;

/**
 * 昼夜循环Actor
 * 放置在场景中，驱动太阳旋转和天空变化
 * 支持暂停/调速
 */
UCLASS()
class DIGITALTWINPARK_API ADTPDayNightCycle : public AActor
{
	GENERATED_BODY()

public:
	ADTPDayNightCycle();

	/** 设置时间（0-24小时） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|DayNight")
	void SetTimeOfDay(float Hour);

	/** 获取当前时间 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|DayNight")
	float GetTimeOfDay() const { return CurrentHour; }

	/** 暂停/恢复循环 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|DayNight")
	void SetPaused(bool bPaused);

	/** 设置循环速度倍率（1.0=实时, 60.0=1分钟=1小时） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|DayNight")
	void SetTimeScale(float Scale);

	// ========================================================================
	// 配置
	// ========================================================================

	/** 太阳光 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|DayNight|Lighting")
	TObjectPtr<UDirectionalLightComponent> SunLight;

	/** 天空光 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|DayNight|Lighting")
	TObjectPtr<USkyLightComponent> SkyLight;

	/** 日出时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|DayNight")
	float SunriseHour = 6.0f;

	/** 日落时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|DayNight")
	float SunsetHour = 18.0f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	/** 根据当前时间更新太阳角度 */
	void UpdateSunPosition();

	float CurrentHour = 12.0f;
	float TimeScale = 60.0f; // 默认1分钟=1小时
	bool bIsPaused = false;
};