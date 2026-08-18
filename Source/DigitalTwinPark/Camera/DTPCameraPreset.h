// DigitalTwinPark - 智慧园区数字孪生
// 相机预设DataAsset - 编辑器配置视角

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DTPCameraPreset.generated.h"

/**
 * 相机预设
 * 在编辑器中创建多个预设视角（鸟瞰、建筑特写等）
 * 大屏触控通过切换预设视角来快速导航
 */
UCLASS(BlueprintType)
class DIGITALTWINPARK_API UDTPCameraPreset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 预设名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Camera")
	FText PresetName;

	/** 相机位置 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Camera")
	FVector Location;

	/** 相机旋转 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Camera")
	FRotator Rotation;

	/** 飞行过渡时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Camera")
	float TransitionTime = 1.0f;

	/** 缩略图（可选） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DigitalTwinPark|Camera")
	TObjectPtr<UTexture2D> Thumbnail;
};