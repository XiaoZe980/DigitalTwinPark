// DigitalTwinPark - 智慧园区数字孪生
// 夜间灯光管理器实现

#include "DTPNightLightManager.h"
#include "DigitalTwinPark.h"
#include "EngineUtils.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

ADTPNightLightManager::ADTPNightLightManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ADTPNightLightManager::BeginPlay()
{
	Super::BeginPlay();

	CollectLights();
	ApplyNightMode(false); // 初始按白天状态

	UE_LOG(LogDTP, Log, TEXT("[DTP] NightLightManager 接管 %d 个灯具（自动接管: %s / 模板: %s）"),
		LightActors.Num(),
		bControlAllPointLights ? TEXT("开") : TEXT("关"),
		LightTemplateActor ? *LightTemplateActor->GetName() : TEXT("无"));

	// 打印前若干个被接管的 Actor，便于核对是否误接管
	for (int32 Index = 0; Index < LightActors.Num() && Index < 10; ++Index)
	{
		if (LightActors[Index])
		{
			UE_LOG(LogDTP, Log, TEXT("[DTP]   灯具[%d]: %s"), Index, *LightActors[Index]->GetName());
		}
	}
	if (LightActors.Num() > 10)
	{
		UE_LOG(LogDTP, Log, TEXT("[DTP]   ...（其余 %d 个略）"), LightActors.Num() - 10);
	}
}

void ADTPNightLightManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bAutoFollowSky) return;

	// 每 0.5 秒检查一次天空时间即可
	CheckTimer += DeltaTime;
	if (CheckTimer < 0.5f) return;
	CheckTimer = 0.0f;

	const float Hour = ReadSkyHour();
	if (Hour < 0.0f) return; // 场景没有 UDS，保持手动控制

	// 跨午夜判断：Hour >= 18 或 Hour < 6 为夜间
	const bool bShouldBeNight = (Hour >= NightStartHour || Hour < NightEndHour);
	if (bShouldBeNight != bIsNight)
	{
		ApplyNightMode(bShouldBeNight);
	}
}

float ADTPNightLightManager::ReadSkyHour() const
{
	if (!GetWorld()) return -1.0f;

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || !Actor->GetClass()->GetName().Contains(TEXT("Ultra_Dynamic_Sky")))
		{
			continue;
		}

		// UDS 的 "Time of Day" 属性值为 0-2400（可能存为 double 或 float）
		static const FName TimePropName(TEXT("Time of Day"));

		if (FDoubleProperty* DoubleProp = FindFProperty<FDoubleProperty>(Actor->GetClass(), TimePropName))
		{
			return static_cast<float>(DoubleProp->GetPropertyValue_InContainer(Actor)) / 100.0f;
		}
		if (FFloatProperty* FloatProp = FindFProperty<FFloatProperty>(Actor->GetClass(), TimePropName))
		{
			return FloatProp->GetPropertyValue_InContainer(Actor) / 100.0f;
		}

		UE_LOG(LogDTP, Warning, TEXT("[DTP] 未找到 UDS 的 Time of Day 属性"));
		break;
	}
	return -1.0f;
}

void ADTPNightLightManager::CollectLights()
{
	LightActors.Empty();

	if (!GetWorld()) return;

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor == this) continue;

		// 自动接管模式：收集所有带点光源的 Actor（路灯与光源分开摆放时免打标签）
		if (bControlAllPointLights)
		{
			TArray<UPointLightComponent*> PointLights;
			Actor->GetComponents<UPointLightComponent>(PointLights);
			if (PointLights.Num() > 0)
			{
				LightActors.Add(Actor);
				continue;
			}
		}

		// 标签模式：收集带标签的 Actor（灯罩材质切换 + 自动补光）
		if (Actor->ActorHasTag(LightActorTag))
		{
			EnsureLightComponent(Actor);
			LightActors.Add(Actor);
		}
	}
}

void ADTPNightLightManager::EnsureLightComponent(AActor* LightActor)
{
	if (!bAutoCreateLights || !LightActor || !LightActor->GetRootComponent())
	{
		return;
	}

	// 已有光源组件则不动
	TArray<UPointLightComponent*> Existing;
	LightActor->GetComponents<UPointLightComponent>(Existing);
	if (Existing.Num() > 0)
	{
		return;
	}

	// 创建点光源：有模板则整体复制组件，保证所有参数（含强度单位/衰减/IES）完全一致
	// 注意：逐个 Set 参数会丢失单位语义——同样数值在 Candelas 与 Unitless 下亮度天差地别
	UPointLightComponent* Template = FindTemplateLight();
	UPointLightComponent* NewLight = nullptr;

	if (Template)
	{
		NewLight = DuplicateObject<UPointLightComponent>(Template, LightActor);
	}
	else
	{
		NewLight = NewObject<UPointLightComponent>(LightActor);
		if (NewLight)
		{
			NewLight->SetIntensity(AutoLightIntensity);
			NewLight->SetAttenuationRadius(AutoLightRadius);
			NewLight->SetLightColor(AutoLightColor);
		}
	}
	if (!NewLight) return;

	NewLight->SetupAttachment(LightActor->GetRootComponent());
	NewLight->SetRelativeLocation(AutoLightOffset);
	NewLight->SetCastShadows(false);
	NewLight->SetMobility(EComponentMobility::Movable);
	NewLight->RegisterComponent();
	LightActor->AddInstanceComponent(NewLight);

	UE_LOG(LogDTP, Log, TEXT("[DTP] 已为路灯 %s 自动创建点光源%s（强度=%.1f 单位=%d 半径=%.0f）"),
		*LightActor->GetName(), Template ? TEXT("（复制模板）") : TEXT("（默认参数）"),
		NewLight->Intensity, static_cast<int32>(NewLight->IntensityUnits), NewLight->AttenuationRadius);
}

UPointLightComponent* ADTPNightLightManager::FindTemplateLight() const
{
	if (!LightTemplateActor) return nullptr;

	TArray<UPointLightComponent*> TemplateLights;
	LightTemplateActor->GetComponents<UPointLightComponent>(TemplateLights);
	return TemplateLights.Num() > 0 ? TemplateLights[0] : nullptr;
}

void ADTPNightLightManager::RefreshLights()
{
	CollectLights();
	ApplyNightMode(bIsNight); // 新收集的灯具同步到当前状态
	UE_LOG(LogDTP, Log, TEXT("[DTP] NightLightManager 刷新: %d 个路灯"), LightActors.Num());
}

void ADTPNightLightManager::SetNightMode(bool bNight)
{
	// 手动控制（会停止自动跟随，避免被下一次轮询覆盖）
	bAutoFollowSky = false;
	ApplyNightMode(bNight);
}

void ADTPNightLightManager::ApplyNightMode(bool bNight)
{
	bIsNight = bNight;

	for (AActor* LightActor : LightActors)
	{
		if (!LightActor) continue;

		// 1. 光源开关：点亮/熄灭该 Actor 下所有点光源
		TArray<UPointLightComponent*> PointLights;
		LightActor->GetComponents<UPointLightComponent>(PointLights);
		for (UPointLightComponent* PointLight : PointLights)
		{
			if (!PointLight) continue;

			PointLight->SetVisibility(bNight);

			// 路灯不投影：数量多时会把虚拟阴影贴图(VSM)撑爆，导致光照渲染异常
			if (bDisableLightShadows && PointLight->CastShadows)
			{
				PointLight->SetCastShadows(false);
			}
		}

		// 2. 灯罩材质切换：仅对打了标签的 Actor（避免误改其他含光源的网格）
		UMaterialInterface* TargetMaterial = bNight ? NightMaterial : DayMaterial;
		if (TargetMaterial && LightActor->ActorHasTag(LightActorTag))
		{
			TArray<UStaticMeshComponent*> Meshes;
			LightActor->GetComponents<UStaticMeshComponent>(Meshes);
			for (UStaticMeshComponent* Mesh : Meshes)
			{
				if (Mesh && Mesh->GetNumMaterials() > MaterialSlotIndex)
				{
					Mesh->SetMaterial(MaterialSlotIndex, TargetMaterial);
				}
			}
		}
	}

	UE_LOG(LogDTP, Log, TEXT("[DTP] 路灯切换: %s（%d 个灯具）"),
		bNight ? TEXT("夜间亮灯") : TEXT("白天熄灯"), LightActors.Num());

	// 打印首个光源的实际状态，便于排查"灯不亮"的问题
	if (LightActors.Num() > 0 && LightActors[0])
	{
		TArray<UPointLightComponent*> DebugLights;
		LightActors[0]->GetComponents<UPointLightComponent>(DebugLights);
		if (DebugLights.Num() > 0 && DebugLights[0])
		{
			UPointLightComponent* L = DebugLights[0];
			UE_LOG(LogDTP, Log, TEXT("[DTP] 光源状态样例[%s]: 强度=%.1f 单位=%d 半径=%.0f 可见=%d 相对位置=%s 世界位置=%s"),
				*L->GetName(), L->Intensity, static_cast<int32>(L->IntensityUnits),
				L->AttenuationRadius, L->IsVisible() ? 1 : 0,
				*L->GetRelativeLocation().ToString(), *L->GetComponentLocation().ToString());
		}
		else
		{
			UE_LOG(LogDTP, Warning, TEXT("[DTP] 灯具 %s 上没有点光源组件"), *LightActors[0]->GetName());
		}
	}
}
