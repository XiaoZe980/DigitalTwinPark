// DigitalTwinPark - 智慧园区数字孪生
// 昼夜循环实现

#include "DTPDayNightCycle.h"
#include "DigitalTwinPark.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"

ADTPDayNightCycle::ADTPDayNightCycle()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ADTPDayNightCycle::BeginPlay()
{
	Super::BeginPlay();
	UpdateSunPosition();
}

void ADTPDayNightCycle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsPaused)
	{
		// 推进时间
		CurrentHour += (DeltaTime / 3600.0f) * TimeScale;
		if (CurrentHour >= 24.0f)
		{
			CurrentHour -= 24.0f;
		}

		UpdateSunPosition();
	}
}

void ADTPDayNightCycle::SetTimeOfDay(float Hour)
{
	CurrentHour = FMath::Clamp(Hour, 0.0f, 24.0f);
	UpdateSunPosition();
}

void ADTPDayNightCycle::SetPaused(bool bPaused)
{
	bIsPaused = bPaused;
}

void ADTPDayNightCycle::SetTimeScale(float Scale)
{
	TimeScale = FMath::Max(0.0f, Scale);
}

void ADTPDayNightCycle::UpdateSunPosition()
{
	if (!SunLight) return;

	// 将时间映射到太阳角度（0°=日出, 180°=日落）
	// 6点=日出(0°), 12点=正午(90°), 18点=日落(180°), 24点=午夜(270°)
	float SunAngle = 0.0f;

	if (CurrentHour >= SunriseHour && CurrentHour <= SunsetHour)
	{
		// 白天：从地平线升到正午再降到地平线
		float DayProgress = (CurrentHour - SunriseHour) / (SunsetHour - SunriseHour);
		SunAngle = DayProgress * 180.0f;
	}
	else
	{
		// 夜晚：在地平线以下
		float NightProgress;
		if (CurrentHour > SunsetHour)
		{
			NightProgress = (CurrentHour - SunsetHour) / (24.0f - SunsetHour + SunriseHour);
		}
		else
		{
			NightProgress = (CurrentHour + 24.0f - SunsetHour) / (24.0f - SunsetHour + SunriseHour);
		}
		SunAngle = 180.0f + NightProgress * 180.0f;
	}

	// 设置太阳旋转（绕Y轴旋转）
	FRotator SunRotation(SunAngle - 90.0f, 0.0f, 0.0f);
	SunLight->SetWorldRotation(SunRotation);

	// 根据太阳角度调整光照强度
	float SunIntensity = FMath::Sin(FMath::DegreesToRadians(SunAngle));
	SunIntensity = FMath::Clamp(SunIntensity * 10.0f, 0.5f, 10.0f);
	SunLight->SetIntensity(SunIntensity);
}