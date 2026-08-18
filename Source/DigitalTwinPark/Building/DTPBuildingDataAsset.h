// DigitalTwinPark - 智慧园区数字孪生
// 建筑配置DataAsset - 在编辑器中配置建筑属性

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DTPBuildingDataAsset.generated.h"

/**
 * 建筑配置数据资产
 * 每栋建筑创建一个实例，在编辑器中拖拽到BuildingActor上
 */
UCLASS(BlueprintType)
class DIGITALTWINPARK_API UDTPBuildingDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 建筑ID（唯一标识，与IoT数据对应） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Building")
	FString BuildingId;

	/** 建筑名称（显示用） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Building")
	FString BuildingName;

	/** 楼层数 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Building")
	int32 FloorCount = 1;

	/** 建筑面积（平方米） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Building")
	float Area = 1000.0f;

	/** 建筑类型标签 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Building")
	FString BuildingType;

	/** 建筑描述 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Building")
	FText Description;
};