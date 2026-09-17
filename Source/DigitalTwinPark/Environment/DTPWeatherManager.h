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

	// ========================================================================
	// Ultra Dynamic Weather 联动（天气数据 → 真实天气视觉）
	// ========================================================================

	/** 是否驱动场景中的 Ultra Dynamic Weather（留空预设则自动按默认路径加载） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Weather|UDW")
	bool bDriveUDW = true;

	/** 晴天预设（留空自动加载 Clear_Skies） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Weather|UDW")
	TObjectPtr<UObject> SunnyPreset;

	/** 多云预设（留空自动加载 Partly_Cloudy） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Weather|UDW")
	TObjectPtr<UObject> CloudyPreset;

	/** 雨天预设（留空自动加载 Rain） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Weather|UDW")
	TObjectPtr<UObject> RainyPreset;

	/** 雪天预设（留空自动加载 Snow） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Weather|UDW")
	TObjectPtr<UObject> SnowyPreset;

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

	/** 数据刷新回调 → 把最新天气数据应用到场景 */
	UFUNCTION()
	void HandleWeatherDataUpdated();

	/** 按天气类型取对应预设（未配置时按默认路径加载） */
	UObject* ResolvePreset(EDTPWeatherType Weather);

	/** 反射调用场景中 UDW 的 Change Weather，驱动真实天气视觉 */
	void ApplyWeatherToUDW(EDTPWeatherType Weather);

	EDTPWeatherType CurrentWeather = EDTPWeatherType::Sunny;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> ActiveWeatherEffect;
};