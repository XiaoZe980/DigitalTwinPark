// DigitalTwinPark - 智慧园区数字孪生
// GameInstance - 全局状态管理，子系统注册

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DTPGameInstance.generated.h"

/**
 * 全局游戏实例
 * 持有UDTPDataSubsystem等跨关卡子系统
 * 子系统在Initialize中自动创建
 */
UCLASS()
class DIGITALTWINPARK_API UDTPGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	/** 获取数据子系统 */
	UFUNCTION(BlueprintCallable, Category = "DigitalTwinPark")
	class UDTPDataSubsystem* GetDataSubsystem() const;
};