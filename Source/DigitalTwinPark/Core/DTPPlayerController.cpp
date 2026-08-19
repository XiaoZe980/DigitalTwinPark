// DigitalTwinPark - 智慧园区数字孪生
// PlayerController实现

#include "DTPPlayerController.h"
#include "DTPPawn.h"
#include "Building/DTPBuildingActor.h"
#include "Data/DTPDataTypes.h"
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

	UE_LOG(LogDTP, Log, TEXT("[DTP] PlayerController BeginPlay"));
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