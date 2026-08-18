// DigitalTwinPark - 智慧园区数字孪生
// GameInstance实现

#include "DTPGameInstance.h"
#include "Data/DTPDataSubsystem.h"
#include "DigitalTwinPark.h"

void UDTPGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogDTP, Log, TEXT("[DTP] GameInstance Init"));
}

void UDTPGameInstance::Shutdown()
{
	UE_LOG(LogDTP, Log, TEXT("[DTP] GameInstance Shutdown"));
	Super::Shutdown();
}

UDTPDataSubsystem* UDTPGameInstance::GetDataSubsystem() const
{
	return GetSubsystem<UDTPDataSubsystem>();
}