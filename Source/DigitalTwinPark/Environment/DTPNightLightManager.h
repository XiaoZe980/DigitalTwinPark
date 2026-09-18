// DigitalTwinPark - 智慧园区数字孪生
// 夜间灯光管理器 - 路灯随天空时间自动亮灭

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DTPNightLightManager.generated.h"

class UMaterialInterface;
class UPointLightComponent;

/**
 * 夜间灯光管理器
 * 放置在场景中，按 Ultra Dynamic Sky 的当前时间自动控制路灯亮灭
 * 同时切换灯罩材质（白天透明 / 夜晚自发光）
 *
 * 用法：给场景中的路灯 Actor 打上 LightActorTag 标签（可多选批量加），
 * 管理器会自动收集并统一控制
 */
UCLASS()
class DIGITALTWINPARK_API ADTPNightLightManager : public AActor
{
	GENERATED_BODY()

public:
	ADTPNightLightManager();

	/** 手动设置昼夜状态（不依赖 UDS 时可直接调用） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|NightLight")
	void SetNightMode(bool bNight);

	/** 当前是否夜间状态 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|NightLight")
	bool IsNightMode() const { return bIsNight; }

	/** 重新收集场景中的路灯 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|NightLight")
	void RefreshLights();

	// ========================================================================
	// 配置
	// ========================================================================

	/** 自动接管场景中所有点光源（免打标签，适合路灯与光源分开摆放的情况） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight")
	bool bControlAllPointLights = true;

	/** 路灯标签：带此标签的 Actor 会被控制材质切换（光源已由上面自动接管） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight")
	FName LightActorTag = TEXT("NightLight");

	/** 夜晚灯罩材质（自发光），留空则不切换材质 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight")
	TObjectPtr<UMaterialInterface> NightMaterial;

	/** 白天灯罩材质（透明），留空则不切换材质 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight")
	TObjectPtr<UMaterialInterface> DayMaterial;

	/** 灯罩所在的材质槽索引（资产包路灯的灯罩在 Element 1） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight")
	int32 MaterialSlotIndex = 1;

	/** 天黑时刻（小时，0-24） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight")
	float NightStartHour = 18.0f;

	/** 天亮时刻（小时，0-24） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight")
	float NightEndHour = 6.0f;

	/** 是否自动跟随天空时间（关闭则只能手动 SetNightMode） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight")
	bool bAutoFollowSky = true;

	/** 关闭路灯的阴影投射（路灯不需要投影，可避免大量点光源撑爆虚拟阴影贴图 VSM） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight")
	bool bDisableLightShadows = true;

	// ========================================================================
	// 自动补光（路灯只有网格、没有光源组件时自动创建）
	// ========================================================================

	/** 给没有光源组件的路灯自动创建点光源 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight|AutoLight")
	bool bAutoCreateLights = true;

	/** 光源模板 Actor：自动创建的光源会复制它身上点光源的参数（强度/半径/颜色等）。留空则用下方默认值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight|AutoLight")
	TObjectPtr<AActor> LightTemplateActor;

	/** 自动创建的光源相对路灯的位置（按灯头高度调，Z 为高度） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight|AutoLight")
	FVector AutoLightOffset = FVector(0.0f, 0.0f, 600.0f);

	/** 自动光源强度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight|AutoLight")
	float AutoLightIntensity = 3000.0f;

	/** 自动光源照射半径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight|AutoLight")
	float AutoLightRadius = 1200.0f;

	/** 自动光源颜色（默认暖黄路灯色） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|NightLight|AutoLight")
	FLinearColor AutoLightColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	/** 读取 UDS 当前小时（0-24），读不到返回 -1 */
	float ReadSkyHour() const;

	/** 把昼夜状态应用到所有路灯 */
	void ApplyNightMode(bool bNight);

	/** 收集带标签的路灯 Actor */
	void CollectLights();

	/** 给路灯补上点光源（已有光源则跳过） */
	void EnsureLightComponent(AActor* LightActor);

	/** 取模板 Actor 上的点光源组件（无则返回 nullptr） */
	UPointLightComponent* FindTemplateLight() const;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> LightActors;

	bool bIsNight = false;
	float CheckTimer = 0.0f;
};
