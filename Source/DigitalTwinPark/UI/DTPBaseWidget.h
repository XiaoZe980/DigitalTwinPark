// DigitalTwinPark - 智慧园区数字孪生
// UI基类 - 所有Widget继承此基类

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DTPBaseWidget.generated.h"

/**
 * 数字孪生Widget基类
 * 提供统一的初始化流程和数据绑定接口
 */
UCLASS(abstract)
class DIGITALTWINPARK_API UDTPBaseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 初始化Widget（由外部调用，替代BeginPlay） */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|UI")
	virtual void InitializeWidget();

	/** 刷新Widget数据 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark|UI")
	virtual void RefreshData();

	/** 获取数据子系统 */
	UFUNCTION(BlueprintPure, Category = "DigitalTwinPark|UI")
	class UDTPDataSubsystem* GetDataSubsystem() const;

protected:
	virtual void NativeConstruct() override;

	/** 数据子系统缓存 */
	UPROPERTY()
	TObjectPtr<class UDTPDataSubsystem> DataSubsystem;

	/** 是否已初始化 */
	bool bIsInitialized = false;
};