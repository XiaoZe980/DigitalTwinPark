// DigitalTwinPark - 智慧园区数字孪生
// 输入处理器实现

#include "DTPInputHandler.h"
#include "DigitalTwinPark.h"
#include "Misc/EngineVersion.h"

ADTPInputHandler::ADTPInputHandler()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADTPInputHandler::BeginPlay()
{
	Super::BeginPlay();
	DetectPlatform();
}

void ADTPInputHandler::SetTouchMode(bool bTouchMode)
{
	bIsTouchMode = bTouchMode;
	UE_LOG(LogDTP, Log, TEXT("[DTP] 输入模式: %s"), bTouchMode ? TEXT("触控") : TEXT("键鼠"));
}

bool ADTPInputHandler::IsMobilePlatform() const
{
#if PLATFORM_ANDROID || PLATFORM_IOS
	return true;
#else
	return false;
#endif
}

void ADTPInputHandler::DetectPlatform()
{
	if (IsMobilePlatform())
	{
		SetTouchMode(true);
	}
	else
	{
		SetTouchMode(false);
	}
}