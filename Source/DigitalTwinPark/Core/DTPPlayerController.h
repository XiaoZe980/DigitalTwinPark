// DigitalTwinPark - 智慧园区数字孪生
// PlayerController - 输入处理、相机控制、建筑选择

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DTPPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class ADTPBuildingActor;
class UDTPHUDWidget;
class UDTPCameraPreset;
class ADTPBuildingManager;
class ADTPDayNightCycle;

/**
 * 数字孪生玩家控制器
 * 处理多平台输入（桌面鼠标键盘 + 触控手势）
 * 管理建筑选择（点击高亮）和相机模式切换
 */
UCLASS()
class DIGITALTWINPARK_API ADTPPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADTPPlayerController();

	// ========================================================================
	// 建筑选择
	// ========================================================================

	/** 当前选中的建筑 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Selection")
	ADTPBuildingActor* GetSelectedBuilding() const { return SelectedBuilding; }

	/** 选择建筑 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Selection")
	void SelectBuilding(ADTPBuildingActor* Building);

	/** 取消选择 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Selection")
	void DeselectBuilding();

	/** 建筑选中变化委托 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDTPOnBuildingSelected, ADTPBuildingActor*, Building);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDTPOnBuildingDeselected);

	UPROPERTY(BlueprintAssignable, Category = "DigitalTwinPark|Selection")
	FDTPOnBuildingSelected OnBuildingSelected;

	UPROPERTY(BlueprintAssignable, Category = "DigitalTwinPark|Selection")
	FDTPOnBuildingDeselected OnBuildingDeselected;

	// ========================================================================
	// 输入配置
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DigitalTwinPark|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DigitalTwinPark|Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DigitalTwinPark|Input")
	TObjectPtr<UInputAction> IA_MoveUpDown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DigitalTwinPark|Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DigitalTwinPark|Input")
	TObjectPtr<UInputAction> IA_Zoom;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DigitalTwinPark|Input")
	TObjectPtr<UInputAction> IA_Click;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DigitalTwinPark|Input")
	TObjectPtr<UInputAction> IA_TouchTap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DigitalTwinPark|Input")
	TObjectPtr<UInputAction> IA_TouchPinch;

	// ========================================================================
	// UI
	// ========================================================================

	/** HUD Widget 蓝图类（默认加载 /Game/UI/WBP_HUD） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DigitalTwinPark|UI")
	TSubclassOf<UDTPHUDWidget> HUDWidgetClass;

	/** 当前 HUD Widget 实例 */
	UPROPERTY()
	TObjectPtr<UDTPHUDWidget> HUDWidget;

	/** 关闭建筑信息卡片（取消显示状态，数据刷新不再弹出） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|UI")
	void CloseBuildingInfoCard();

	/** 飞行到指定相机预设（HUD 预设按钮调用，转发到 Pawn 的相机管理器） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Camera")
	void FlyToCameraPreset(UDTPCameraPreset* Preset);

	/** 相机聚焦到指定建筑（HUD 聚焦按钮调用，转发到 Pawn 的相机管理器） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Camera")
	void FocusOnBuilding(ADTPBuildingActor* Building);

	/** 聚焦到指定索引告警对应的建筑（点击告警弹窗某条时调用，0 = 第一条） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Camera")
	void FocusOnAlertBuilding(int32 AlertIndex);

	/** 开关建筑热力变色（HUD 按钮调用，内部查找场景中的 BuildingManager） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Building")
	void SetHeatMapEnabled(bool bEnabled);

	/** 告警联动聚焦开关（默认关闭，避免游览中突然跳转打断体验） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DigitalTwinPark|Camera")
	bool bEnableAlertFocus = false;

	/** 设置告警联动聚焦开关 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Camera")
	void SetAlertFocusEnabled(bool bEnabled);

	/** 切换白天/夜晚（驱动场景中的 DTPDayNightCycle） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|Environment")
	void SetDayNight(bool bNight);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	// ========================================================================
	// 输入回调
	// ========================================================================
	void OnMove(const struct FInputActionValue& Value);
	void OnMoveUpDown(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnZoom(const FInputActionValue& Value);
	void OnClick();
	void OnTouchTap(const FInputActionValue& Value);
	void OnTouchPinch(const FInputActionValue& Value);

private:
	/** 鼠标点击射线检测，选择建筑 */
	void PerformClickTrace();

	/** 触控点击射线检测 */
	void PerformTouchTrace(const FVector2D& ScreenPosition);

	/** 从资源路径加载所有 Input Action 和 IMC（不依赖蓝图配置） */
	void LoadInputAssets();

	/** 创建 HUD 并绑定建筑选中事件 */
	void SetupHUD();

	/** 建筑选中回调 → 显示建筑信息 */
	UFUNCTION()
	void HandleBuildingSelected(ADTPBuildingActor* Building);

	/** 取消选择回调 → 隐藏建筑信息 */
	UFUNCTION()
	void HandleBuildingDeselected();

	/** 数据刷新回调 → 卡片可见时用最新数据刷新 */
	UFUNCTION()
	void HandleDataUpdated();

	/** 告警刷新回调 → 新告警出现时相机聚焦到对应建筑 */
	UFUNCTION()
	void HandleAlertUpdated();

	/** 查找场景中的建筑管理器 */
	ADTPBuildingManager* FindBuildingManager() const;

	/** 当前选中建筑 */
	UPROPERTY()
	TObjectPtr<ADTPBuildingActor> SelectedBuilding;

	/** 已处理过（聚焦过）的告警 ID，避免重复聚焦 */
	TArray<FString> HandledAlertIds;

	/** 上次告警聚焦时间（冷却，避免每2秒刷新都聚焦太吵） */
	float LastAlertFocusTime = -100.0f;

	/** 鼠标位置缓存 */
	FVector2D LastMousePosition;

	/** 是否正在拖拽 */
	bool bIsDragging = false;

	/** 建筑信息卡片是否可见（关闭按钮会置 false） */
	bool bInfoCardVisible = false;
};