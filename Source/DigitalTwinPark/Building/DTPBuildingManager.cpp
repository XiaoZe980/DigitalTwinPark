// DigitalTwinPark - 智慧园区数字孪生
// 建筑管理器实现

#include "DTPBuildingManager.h"
#include "DTPBuildingActor.h"
#include "Data/DTPDataSubsystem.h"
#include "Data/DTPDataTypes.h"
#include "DigitalTwinPark.h"
#include "EngineUtils.h"
#include "Engine/GameInstance.h"

ADTPBuildingManager::ADTPBuildingManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADTPBuildingManager::BeginPlay()
{
	Super::BeginPlay();
	RefreshBuildings();
	UE_LOG(LogDTP, Log, TEXT("[DTP] BuildingManager 注册了 %d 栋建筑"), RegisteredBuildings.Num());

	// 订阅数据更新，把实时数据同步到每栋建筑
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDTPDataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UDTPDataSubsystem>())
		{
			DataSubsystem->OnBuildingDataUpdated.AddDynamic(this, &ADTPBuildingManager::HandleBuildingDataUpdated);
			UE_LOG(LogDTP, Log, TEXT("[DTP] BuildingManager 已订阅数据更新"));
		}
	}
}

void ADTPBuildingManager::HandleBuildingDataUpdated()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;
	UDTPDataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UDTPDataSubsystem>();
	if (!DataSubsystem) return;

	// 用最新缓存数据更新每栋建筑
	for (ADTPBuildingActor* Building : RegisteredBuildings)
	{
		if (!Building) continue;

		const FString BuildingId = Building->GetBuildingData().BuildingId;
		if (BuildingId.IsEmpty()) continue;

		FDTPBuildingData MatchingData;
		if (DataSubsystem->FindBuildingById(BuildingId, MatchingData))
		{
			Building->UpdateData(MatchingData);
		}
	}
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