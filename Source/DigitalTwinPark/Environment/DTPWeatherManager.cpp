// DigitalTwinPark - 智慧园区数字孪生
// 天气管理器实现

#include "DTPWeatherManager.h"
#include "DigitalTwinPark.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

ADTPWeatherManager::ADTPWeatherManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADTPWeatherManager::BeginPlay()
{
	Super::BeginPlay();
	SetWeather(EDTPWeatherType::Sunny);
}

void ADTPWeatherManager::SetWeather(EDTPWeatherType NewWeather)
{
	if (CurrentWeather == NewWeather) return;

	CurrentWeather = NewWeather;

	// 激活粒子效果
	ActivateWeatherEffect(NewWeather);

	// 通知蓝图
	OnWeatherChanged(NewWeather);

	UE_LOG(LogDTP, Log, TEXT("[DTP] 天气切换: %d"), static_cast<uint8>(NewWeather));
}

void ADTPWeatherManager::UpdateWeatherData(const FDTPWeatherData& Data)
{
	SetWeather(Data.WeatherType);
}

void ADTPWeatherManager::ActivateWeatherEffect(EDTPWeatherType Weather)
{
	ClearWeatherEffects();

	UNiagaraSystem* EffectToSpawn = nullptr;

	switch (Weather)
	{
	case EDTPWeatherType::Rainy:
		EffectToSpawn = RainEffect;
		break;
	case EDTPWeatherType::Snowy:
		EffectToSpawn = SnowEffect;
		break;
	default:
		break;
	}

	if (EffectToSpawn)
	{
		ActiveWeatherEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
			EffectToSpawn,
			GetRootComponent(),
			NAME_None,
			FVector(0.0f, 0.0f, 500.0f),
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}
}

void ADTPWeatherManager::ClearWeatherEffects()
{
	if (ActiveWeatherEffect)
	{
		ActiveWeatherEffect->DestroyComponent();
		ActiveWeatherEffect = nullptr;
	}
}