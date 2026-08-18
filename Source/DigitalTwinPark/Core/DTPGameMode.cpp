// DigitalTwinPark - 智慧园区数字孪生
// GameMode实现

#include "DTPGameMode.h"
#include "DTPPlayerController.h"
#include "DTPPawn.h"
#include "DTPGameState.h"
#include "DTPGameInstance.h"

ADTPGameMode::ADTPGameMode()
{
	// 设置默认类
	PlayerControllerClass = ADTPPlayerController::StaticClass();
	DefaultPawnClass = ADTPPawn::StaticClass();
	GameStateClass = ADTPGameState::StaticClass();
}