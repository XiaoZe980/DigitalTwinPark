// DigitalTwinPark - 智慧园区数字孪生
// 相机管理器 - 多模式切换、预设视角、飞行动画

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DTPCameraManager.generated.h"

class UDTPCameraPreset;
class USpringArmComponent;
class UCameraComponent;

/** 相机模式 */
UENUM(BlueprintType)
enum class EDTPCameraMode : uint8
{
	FreeRoam	UMETA(DisplayName = "自由漫游"),
	Preset		UMETA(DisplayName = "预设视角"),
	FocusBuilding UMETA(DisplayName = "建筑聚焦"),
	TopDown		UMETA(DisplayName = "鸟瞰"),
};

/**
 * 相机管理器
 * 挂载在PlayerController上，管理多相机模式
 */
UCLASS(ClassGroup = (DigitalTwinPark), meta = (BlueprintSpawnableComponent))
class DIGITALTWINPARK_API UDTPCameraManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UDTPCameraManager();

	// ========================================================================
	// 模式切换
	// ========================================================================

	/** 切换到自由漫游 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Camera")
	void SetFreeRoamMode();

	/** 飞行到预设视角 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Camera")
	void FlyToPreset(UDTPCameraPreset* Preset);

	/** 聚焦建筑 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Camera")
	void FocusOnBuilding(class ADTPBuildingActor* Building);

	/** 切换到鸟瞰 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Camera")
	void SetTopDownMode();

	/** 获取当前模式 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|Camera")
	EDTPCameraMode GetCurrentMode() const { return CurrentMode; }

	// ========================================================================
	// 预设列表
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Camera")
	TArray<TObjectPtr<UDTPCameraPreset>> Presets;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** 执行飞行动画每帧更新 */
	void UpdateFlyTo(float DeltaTime);

	EDTPCameraMode CurrentMode = EDTPCameraMode::FreeRoam;

	// 飞行过渡
	bool bIsFlying = false;
	FVector FlyStartLocation;
	FRotator FlyStartRotation;
	FVector FlyTargetLocation;
	FRotator FlyTargetRotation;
	float FlyDuration = 1.0f;
	float FlyElapsed = 0.0f;

	// 缓存引用
	UPROPERTY()
	TObjectPtr<USpringArmComponent> CachedSpringArm;

	UPROPERTY()
	TObjectPtr<UCameraComponent> CachedCamera;
};