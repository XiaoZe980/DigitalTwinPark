// DigitalTwinPark - 智慧园区数字孪生
// 建筑管理器实现

#include "DTPBuildingManager.h"
#include "DTPBuildingActor.h"
#include "Data/DTPDataTypes.h"
#include "DigitalTwinPark.h"
#include "EngineUtils.h"

ADTPBuildingManager::ADTPBuildingManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADTPBuildingManager::BeginPlay()
{
	Super::BeginPlay();
	RefreshBuildings();
	UE_LOG(LogDTP, Log, TEXT("[DTP] BuildingManager 注册了 %d 栋建筑"), RegisteredBuildings.Num());
}

void ADTPBuildingManager::RefreshBuildings()
{
	RegisteredBuildings.Empty();

	// 遍历场景中所有BuildingActor
	for (TActorIterator<ADTPBuildingActor> It(GetWorld()); It; ++It)
	{
		RegisteredBuildings.Add(*It);
	}

	UE_LOG(LogDTP, Log, TEXT("[DTP] BuildingManager 刷新: %d 栋建筑"), RegisteredBuildings.Num());
}

ADTPBuildingActor* ADTPBuildingManager::FindBuildingById(const FString& BuildingId) const
{
	for (ADTPBuildingActor* Building : RegisteredBuildings)
	{
		if (Building && Building->GetBuildingData().BuildingId == BuildingId)
		{
			return Building;
		}
	}
	return nullptr;
}

void ADTPBuildingManager::RegisterBuilding(ADTPBuildingActor* Building)
{
	if (Building && !RegisteredBuildings.Contains(Building))
	{
		RegisteredBuildings.Add(Building);
	}
}