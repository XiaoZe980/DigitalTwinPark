// DigitalTwinPark - 智慧园区数字孪生
// PlayerController - 输入处理、相机控制、建筑选择

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DTPPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class ADTPBuildingActor;

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

	/** 当前选中建筑 */
	UPROPERTY()
	TObjectPtr<ADTPBuildingActor> SelectedBuilding;

	/** 鼠标位置缓存 */
	FVector2D LastMousePosition;

	/** 是否正在拖拽 */
	bool bIsDragging = false;
};