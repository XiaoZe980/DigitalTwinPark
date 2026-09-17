// DigitalTwinPark - 智慧园区数字孪生
// 天气管理器实现

#include "DTPWeatherManager.h"
#include "DigitalTwinPark.h"
#include "Data/DTPDataSubsystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "EngineUtils.h"
#include "Engine/GameInstance.h"
#include "UObject/UObjectIterator.h"

ADTPWeatherManager::ADTPWeatherManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADTPWeatherManager::BeginPlay()
{
	Super::BeginPlay();

	// 订阅天气数据更新，让数据自动驱动场景天气
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDTPDataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UDTPDataSubsystem>())
		{
			DataSubsystem->OnWeatherDataUpdated.AddDynamic(this, &ADTPWeatherManager::HandleWeatherDataUpdated);
			UE_LOG(LogDTP, Log, TEXT("[DTP] WeatherManager 已订阅天气数据"));
		}
	}

	// 初始同步到场景（SetWeather 有同值早退，这里强制刷一次让 UDW 也同步）
	ApplyWeatherToUDW(CurrentWeather);
}

void ADTPWeatherManager::HandleWeatherDataUpdated()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;
	UDTPDataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UDTPDataSubsystem>();
	if (!DataSubsystem) return;

	UpdateWeatherData(DataSubsystem->GetWeatherData());
}

void ADTPWeatherManager::SetWeather(EDTPWeatherType NewWeather)
{
	if (CurrentWeather == NewWeather) return;

	CurrentWeather = NewWeather;

	// 激活粒子效果
	ActivateWeatherEffect(NewWeather);

	// 驱动 UDW 真实天气视觉
	ApplyWeatherToUDW(NewWeather);

	// 通知蓝图
	OnWeatherChanged(NewWeather);

	UE_LOG(LogDTP, Log, TEXT("[DTP] 天气切换: %d"), static_cast<uint8>(NewWeather));
}

UObject* ADTPWeatherManager::ResolvePreset(EDTPWeatherType Weather)
{
	// 已配置则直接用；否则按默认路径加载（UDS 天气预设资产）
	TObjectPtr<UObject>* Cached = nullptr;
	const TCHAR* DefaultPath = nullptr;

	switch (Weather)
	{
	case EDTPWeatherType::Sunny:
		Cached = &SunnyPreset;
		DefaultPath = TEXT("/Game/UltraDynamicSky/Blueprints/Weather_Effects/Weather_Presets/Clear_Skies.Clear_Skies");
		break;
	case EDTPWeatherType::Cloudy:
		Cached = &CloudyPreset;
		DefaultPath = TEXT("/Game/UltraDynamicSky/Blueprints/Weather_Effects/Weather_Presets/Partly_Cloudy.Partly_Cloudy");
		break;
	case EDTPWeatherType::Rainy:
		Cached = &RainyPreset;
		DefaultPath = TEXT("/Game/UltraDynamicSky/Blueprints/Weather_Effects/Weather_Presets/Rain.Rain");
		break;
	case EDTPWeatherType::Snowy:
		Cached = &SnowyPreset;
		DefaultPath = TEXT("/Game/UltraDynamicSky/Blueprints/Weather_Effects/Weather_Presets/Snow.Snow");
		break;
	default:
		return nullptr;
	}

	if (!*Cached && DefaultPath)
	{
		*Cached = LoadObject<UObject>(nullptr, DefaultPath);
		if (!*Cached)
		{
			UE_LOG(LogDTP, Warning, TEXT("[DTP] 天气预设加载失败: %s"), DefaultPath);
		}
	}
	return *Cached;
}

void ADTPWeatherManager::ApplyWeatherToUDW(EDTPWeatherType Weather)
{
	if (!bDriveUDW || !GetWorld()) return;

	// 查找场景中的 Ultra_Dynamic_Weather（蓝图类，按类名匹配）
	AActor* UDWActor = nullptr;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->GetClass()->GetName().Contains(TEXT("Ultra_Dynamic_Weather")))
		{
			UDWActor = *It;
			break;
		}
	}
	if (!UDWActor)
	{
		// 场景没有 UDW 时静默跳过（未接入 UDW 也能正常跑）
		return;
	}

	UObject* Preset = ResolvePreset(Weather);
	if (!Preset) return;

	// 反射调用 UDW 的 "Change Weather"（蓝图函数，按类型填参，不依赖参数名）
	UFunction* Func = UDWActor->FindFunction(FName(TEXT("Change Weather")));
	if (!Func)
	{
		UE_LOG(LogDTP, Warning, TEXT("[DTP] UDW 上未找到 Change Weather 函数"));
		return;
	}

	uint8* Params = (uint8*)FMemory::Malloc(Func->ParmsSize);
	FMemory::Memzero(Params, Func->ParmsSize);

	for (TFieldIterator<FProperty> It(Func); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		FProperty* Prop = *It;
		if (Prop->HasAnyPropertyFlags(CPF_ReturnParm)) continue;

		if (FObjectPropertyBase* ObjProp = CastField<FObjectPropertyBase>(Prop))
		{
			ObjProp->SetObjectPropertyValue_InContainer(Params, Preset);
		}
		else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
		{
			FloatProp->SetPropertyValue_InContainer(Params, WeatherTransitionTime);
		}
	}

	UDWActor->ProcessEvent(Func, Params);
	FMemory::Free(Params);

	UE_LOG(LogDTP, Log, TEXT("[DTP] UDW 天气已应用: %s -> %s"),
		*UDWActor->GetName(), *Preset->GetName());
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