// DigitalTwinPark - 智慧园区数字孪生
// 天气管理器 - 天气状态切换、粒子效果

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/DTPDataTypes.h"
#include "DTPWeatherManager.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UDirectionalLightComponent;
class USkyLightComponent;

/**
 * 天气管理器
 * 放置在场景中，驱动天气状态变化
 * 与UDTPDataSubsystem的天气数据联动
 */
UCLASS()
class DIGITALTWINPARK_API ADTPWeatherManager : public AActor
{
	GENERATED_BODY()

public:
	ADTPWeatherManager();

	/** 设置天气类型 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Weather")
	void SetWeather(EDTPWeatherType NewWeather);

	/** 获取当前天气 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|Weather")
	EDTPWeatherType GetCurrentWeather() const { return CurrentWeather; }

	/** 更新天气数据（由DataSubsystem调用） */
	void UpdateWeatherData(const FDTPWeatherData& Data);

	// ========================================================================
	// 配置
	// ========================================================================

	/** 雨粒子系统 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DigitalTwinPark|Weather|Effects")
	TObjectPtr<UNiagaraSystem> RainEffect;

	/** 雪粒子系统 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DigitalTwinPark|Weather|Effects")
	TObjectPtr<UNiagaraSystem> SnowEffect;

	/** 天气切换过渡时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Weather")
	float WeatherTransitionTime = 2.0f;

protected:
	virtual void BeginPlay() override;

	/** 蓝图事件：天气变化时触发 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DigitalTwinPark|Weather")
	void OnWeatherChanged(EDTPWeatherType NewWeather);

private:
	/** 激活天气粒子效果 */
	void ActivateWeatherEffect(EDTPWeatherType Weather);

	/** 清理粒子效果 */
	void ClearWeatherEffects();

	EDTPWeatherType CurrentWeather = EDTPWeatherType::Sunny;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> ActiveWeatherEffect;
};