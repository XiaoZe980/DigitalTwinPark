// DigitalTwinPark - 智慧园区数字孪生
// GameMode实现

#include "DTPGameMode.h"
#include "DTPPlayerController.h"
#include "DTPPawn.h"
#include "DTPGameState.h"
#include "DTPGameInstance.h"
#include "Data/DTPDataSubsystem.h"
#include "Engine/GameInstance.h"

ADTPGameMode::ADTPGameMode()
{
	// 设置默认类
	PlayerControllerClass = ADTPPlayerController::StaticClass();
	DefaultPawnClass = ADTPPawn::StaticClass();
	GameStateClass = ADTPGameState::StaticClass();
}

void ADTPGameMode::BeginPlay()
{
	Super::BeginPlay();

	// World 已就绪，启动数据定时刷新（Mock 数据每周期变化）
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDTPDataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UDTPDataSubsystem>())
		{
			DataSubsystem->StartPeriodicUpdates();
		}
	}
}