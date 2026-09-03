// DigitalTwinPark - 智慧园区数字孪生
// PlayerController实现

#include "DTPPlayerController.h"
#include "DTPPawn.h"
#include "Building/DTPBuildingActor.h"
#include "Building/DTPBuildingManager.h"
#include "Environment/DTPDayNightCycle.h"
#include "Kismet/GameplayStatics.h"
#include "Data/DTPDataTypes.h"
#include "Data/DTPDataSubsystem.h"
#include "UI/DTPHUDWidget.h"
#include "Camera/DTPCameraManager.h"
#include "Camera/DTPCameraPreset.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "DigitalTwinPark.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ADTPPlayerController::ADTPPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ADTPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 加载输入资源
	LoadInputAssets();

	// 添加Enhanced Input映射上下文
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			UE_LOG(LogDTP, Log, TEXT("[DTP] IMC 已绑定: %s"), *DefaultMappingContext->GetName());
		}
		else
		{
			UE_LOG(LogDTP, Error, TEXT("[DTP] 无法加载 IMC_Default!"));
		}
	}

	// 创建 HUD 并绑定建筑选中事件
	SetupHUD();

	UE_LOG(LogDTP, Log, TEXT("[DTP] PlayerController BeginPlay"));
}

void ADTPPlayerController::SetupHUD()
{
	// 加载 HUD 蓝图类（不依赖蓝图配置，路径写死）
	if (!HUDWidgetClass)
	{
		HUDWidgetClass = LoadClass<UDTPHUDWidget>(nullptr, TEXT("/Game/UI/WBP_HUD.WBP_HUD_C"));
	}

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UDTPHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport(0);
			HUDWidget->InitializeWidget();
			UE_LOG(LogDTP, Log, TEXT("[DTP] HUD 已创建: %s"), *HUDWidget->GetName());
		}
	}
	else
	{
		UE_LOG(LogDTP, Error, TEXT("[DTP] 无法加载 HUD 蓝图类 WBP_HUD!"));
	}

	// 绑定建筑选中 → 显示/隐藏信息卡片
	OnBuildingSelected.AddDynamic(this, &ADTPPlayerController::HandleBuildingSelected);
	OnBuildingDeselected.AddDynamic(this, &ADTPPlayerController::HandleBuildingDeselected);

	// 订阅数据刷新 → 卡片可见时自动更新
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDTPDataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UDTPDataSubsystem>())
		{
			DataSubsystem->OnBuildingDataUpdated.AddDynamic(this, &ADTPPlayerController::HandleDataUpdated);
			DataSubsystem->OnAlertDataUpdated.AddDynamic(this, &ADTPPlayerController::HandleAlertUpdated);
		}
	}
}

void ADTPPlayerController::HandleBuildingSelected(ADTPBuildingActor* Building)
{
	bInfoCardVisible = true;
	if (HUDWidget && Building)
	{
		HUDWidget->ShowBuildingInfo(Building->GetBuildingData());
	}
}

void ADTPPlayerController::HandleBuildingDeselected()
{
	bInfoCardVisible = false;
	if (HUDWidget)
	{
		HUDWidget->HideBuildingInfo();
	}
}

void ADTPPlayerController::HandleDataUpdated()
{
	// 卡片可见且有选中建筑时，用最新数据重填卡片（每周期自动更新）
	if (bInfoCardVisible && HUDWidget && SelectedBuilding)
	{
		HUDWidget->ShowBuildingInfo(SelectedBuilding->GetBuildingData());
	}
}

void ADTPPlayerController::HandleAlertUpdated()
{
	// 开关关闭时不做联动（默认关闭，避免游览中被打断）
	if (!bEnableAlertFocus)
	{
		return;
	}

	// 新告警（Critical 优先，其次 Warning）出现时，相机自动聚焦到对应建筑
	if (!GetWorld())
	{
		return;
	}

	// 冷却：8 秒内不重复聚焦，避免每 2 秒刷新都拉镜头太吵
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAlertFocusTime < 8.0f)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UDTPDataSubsystem* Data = GameInstance ? GameInstance->GetSubsystem<UDTPDataSubsystem>() : nullptr;
	if (!Data)
	{
		return;
	}

	// Critical 优先，其次 Warning
	const TArray<FDTPAlertData>& Alerts = Data->GetAlertData();
	for (int32 Priority = 0; Priority < 2; ++Priority)
	{
		const EDTPAlertLevel Wanted = (Priority == 0) ? EDTPAlertLevel::Critical : EDTPAlertLevel::Warning;
		for (const FDTPAlertData& Alert : Alerts)
		{
			if (Alert.Level != Wanted)
			{
				continue;
			}
			// 已聚焦过的告警不重复处理
			if (HandledAlertIds.Contains(Alert.AlertId))
			{
				continue;
			}
			HandledAlertIds.Add(Alert.AlertId);
			// 限制记录量，防止 Mock 随机 ID 无限增长
			if (HandledAlertIds.Num() > 50)
			{
				HandledAlertIds.RemoveAt(0);
			}

			ADTPBuildingManager* Mgr = FindBuildingManager();
			ADTPBuildingActor* Building = Mgr ? Mgr->FindBuildingById(Alert.BuildingId) : nullptr;
			if (Building)
			{
				FocusOnBuilding(Building);
				LastAlertFocusTime = Now;
				UE_LOG(LogDTP, Log, TEXT("[DTP] 告警联动聚焦: %s -> %s"), *Alert.Title, *Alert.BuildingId);
			}
			return;
		}
	}
}

ADTPBuildingManager* ADTPPlayerController::FindBuildingManager() const
{
	if (UWorld* World = GetWorld())
	{
		return Cast<ADTPBuildingManager>(
			UGameplayStatics::GetActorOfClass(World, ADTPBuildingManager::StaticClass()));
	}
	return nullptr;
}

void ADTPPlayerController::CloseBuildingInfoCard()
{
	bInfoCardVisible = false;
	if (HUDWidget)
	{
		HUDWidget->HideBuildingInfo();
	}
}

void ADTPPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 确保输入资源已加载（SetupInputComponent 可能早于 BeginPlay 调用）
	LoadInputAssets();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_Move)
		{
			EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ADTPPlayerController::OnMove);
		}
		if (IA_MoveUpDown)
		{
			EnhancedInput->BindAction(IA_MoveUpDown, ETriggerEvent::Triggered, this, &ADTPPlayerController::OnMoveUpDown);
		}
		if (IA_Look)
		{
			EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ADTPPlayerController::OnLook);
		}
		if (IA_Zoom)
		{
			EnhancedInput->BindAction(IA_Zoom, ETriggerEvent::Triggered, this, &ADTPPlayerController::OnZoom);
		}
		if (IA_Click)
		{
			EnhancedInput->BindAction(IA_Click, ETriggerEvent::Started, this, &ADTPPlayerController::OnClick);
		}
		if (IA_TouchTap)
		{
			EnhancedInput->BindAction(IA_TouchTap, ETriggerEvent::Started, this, &ADTPPlayerController::OnTouchTap);
		}
		if (IA_TouchPinch)
		{
			EnhancedInput->BindAction(IA_TouchPinch, ETriggerEvent::Triggered, this, &ADTPPlayerController::OnTouchPinch);
		}
	}
}

void ADTPPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 检测鼠标拖拽（右键按住时）
	if (IsInputKeyDown(EKeys::RightMouseButton))
	{
		GetMousePosition(LastMousePosition.X, LastMousePosition.Y);
		bIsDragging = true;
	}
	else
	{
		bIsDragging = false;
	}
}

// ============================================================================
// 输入回调
// ============================================================================

void ADTPPlayerController::OnMove(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	if (ADTPPawn* MyPawn = GetPawn<ADTPPawn>())
	{
		MyPawn->MoveForward(Input.Y);
		MyPawn->MoveRight(Input.X);
	}
}

void ADTPPlayerController::OnMoveUpDown(const FInputActionValue& Value)
{
	float Input = Value.Get<float>();
	if (ADTPPawn* MyPawn = GetPawn<ADTPPawn>())
	{
		MyPawn->MoveUp(Input);
	}
}

void ADTPPlayerController::OnLook(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	if (ADTPPawn* MyPawn = GetPawn<ADTPPawn>())
	{
		MyPawn->LookAt(Input.X, Input.Y);
	}
}

void ADTPPlayerController::OnZoom(const FInputActionValue& Value)
{
	float Input = Value.Get<float>();
	if (ADTPPawn* MyPawn = GetPawn<ADTPPawn>())
	{
		MyPawn->Zoom(Input);
	}
}

void ADTPPlayerController::OnClick()
{
	// 左键点击且不在拖拽状态时，执行射线检测
	if (!bIsDragging)
	{
		PerformClickTrace();
	}
}

void ADTPPlayerController::OnTouchTap(const FInputActionValue& Value)
{
	FVector2D ScreenPos = Value.Get<FVector2D>();
	PerformTouchTrace(ScreenPos);
}

void ADTPPlayerController::OnTouchPinch(const FInputActionValue& Value)
{
	float PinchAmount = Value.Get<float>();
	if (ADTPPawn* MyPawn = GetPawn<ADTPPawn>())
	{
		MyPawn->Zoom(PinchAmount);
	}
}

// ============================================================================
// 建筑选择
// ============================================================================

void ADTPPlayerController::SelectBuilding(ADTPBuildingActor* Building)
{
	if (SelectedBuilding == Building) return;

	// 取消旧选择
	if (SelectedBuilding)
	{
		SelectedBuilding->SetHighlighted(false);
	}

	SelectedBuilding = Building;

	if (SelectedBuilding)
	{
		SelectedBuilding->SetHighlighted(true);
		OnBuildingSelected.Broadcast(SelectedBuilding);
		UE_LOG(LogDTP, Log, TEXT("[DTP] 选中建筑: %s"), *SelectedBuilding->GetBuildingData().BuildingName);
	}
}

void ADTPPlayerController::DeselectBuilding()
{
	if (SelectedBuilding)
	{
		SelectedBuilding->SetHighlighted(false);
		OnBuildingDeselected.Broadcast();
		SelectedBuilding = nullptr;
	}
}

void ADTPPlayerController::PerformClickTrace()
{
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.bBlockingHit)
	{
		if (ADTPBuildingActor* Building = Cast<ADTPBuildingActor>(Hit.GetActor()))
		{
			SelectBuilding(Building);
		}
		else
		{
			DeselectBuilding();
		}
	}
	else
	{
		DeselectBuilding();
	}
}

void ADTPPlayerController::PerformTouchTrace(const FVector2D& ScreenPosition)
{
	FHitResult Hit;
	if (GetHitResultAtScreenPosition(ScreenPosition, ECC_Visibility, false, Hit))
	{
		if (ADTPBuildingActor* Building = Cast<ADTPBuildingActor>(Hit.GetActor()))
		{
			SelectBuilding(Building);
		}
	}
}

void ADTPPlayerController::LoadInputAssets()
{
	// 从资源路径加载 Input Actions（不依赖蓝图配置，避免脚本设置丢失）
	if (!IA_Move)
		IA_Move = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Move.IA_Move"));
	if (!IA_MoveUpDown)
		IA_MoveUpDown = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_MoveUpDown.IA_MoveUpDown"));
	if (!IA_Look)
		IA_Look = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Look.IA_Look"));
	if (!IA_Zoom)
		IA_Zoom = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Zoom.IA_Zoom"));
	if (!IA_Click)
		IA_Click = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Click.IA_Click"));
	if (!IA_TouchTap)
		IA_TouchTap = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_TouchTap.IA_TouchTap"));

	// 加载 IMC
	if (!DefaultMappingContext)
		DefaultMappingContext = LoadObject<UInputMappingContext>(
			nullptr, TEXT("/Game/Input/IMC_Default.IMC_Default"));
}

void ADTPPlayerController::FlyToCameraPreset(UDTPCameraPreset* Preset)
{
	// HUD 预设按钮 → 转发到 Pawn 的相机管理器执行飞行动画
	ADTPPawn* MyPawn = Cast<ADTPPawn>(GetPawn());
	if (!MyPawn)
	{
		UE_LOG(LogDTP, Warning, TEXT("[DTP] FlyToCameraPreset: 未找到 Pawn"));
		return;
	}
	if (!MyPawn->CameraManager)
	{
		UE_LOG(LogDTP, Warning, TEXT("[DTP] FlyToCameraPreset: 未找到相机管理器"));
		return;
	}
	MyPawn->CameraManager->FlyToPreset(Preset);
}

void ADTPPlayerController::FocusOnBuilding(ADTPBuildingActor* Building)
{
	// HUD 聚焦按钮 → 转发到 Pawn 的相机管理器执行建筑聚焦动画
	if (!Building)
	{
		UE_LOG(LogDTP, Log, TEXT("[DTP] FocusOnBuilding: 未选中建筑，忽略"));
		return;
	}
	ADTPPawn* MyPawn = Cast<ADTPPawn>(GetPawn());
	if (!MyPawn || !MyPawn->CameraManager)
	{
		UE_LOG(LogDTP, Warning, TEXT("[DTP] FocusOnBuilding: 未找到相机管理器"));
		return;
	}
	MyPawn->CameraManager->FocusOnBuilding(Building);
}

void ADTPPlayerController::SetAlertFocusEnabled(bool bEnabled)
{
	bEnableAlertFocus = bEnabled;
	UE_LOG(LogDTP, Log, TEXT("[DTP] 告警联动聚焦: %s"), bEnabled ? TEXT("开") : TEXT("关"));
}

void ADTPPlayerController::SetDayNight(bool bNight)
{
	// 找到场景中的 DTPDayNightCycle，切换到对应时刻（暂停循环保持该状态）
	if (UWorld* World = GetWorld())
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(World, ADTPDayNightCycle::StaticClass(), Actors);
		for (AActor* Actor : Actors)
		{
			if (ADTPDayNightCycle* Cycle = Cast<ADTPDayNightCycle>(Actor))
			{
				Cycle->SetPaused(true);
				Cycle->SetTimeOfDay(bNight ? 20.0f : 12.0f);
				UE_LOG(LogDTP, Log, TEXT("[DTP] 昼夜切换: %s"), bNight ? TEXT("夜晚") : TEXT("白天"));
				return;
			}
		}
	}
	UE_LOG(LogDTP, Warning, TEXT("[DTP] SetDayNight: 场景中未找到 DTPDayNightCycle"));
}