// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RPlayerControllerBase.h"

#include <Subsystems/RWorldStateSubsystem.h>

#include "AIController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Subsystems/RPartySubsystem.h"
#include "Core/RCharacterBase.h"
#include "Components/RTurnComponent.h"
#include "Interaction/RInteractable.h"
#include "Subsystems/RUILayerController.h"

DECLARE_LOG_CATEGORY_EXTERN(RControllerLOG, Log, All);

DEFINE_LOG_CATEGORY(RControllerLOG);

ARPlayerControllerBase::ARPlayerControllerBase()
{
	bShowMouseCursor = true;
}

void ARPlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	// Initialize HUD, input mode, etc. here
	// Enable mouse
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer()))
	{
		if (BaseMappingContext)
		{
			Subsystem->AddMappingContext(BaseMappingContext, 0);
		}
		if (ExplorationMappingContext)
		{
			Subsystem->AddMappingContext(ExplorationMappingContext, 1);
		}
	}


	ZoomCamera({}); // To reset the camera Zoom
	CachedDestinationMarker = GetWorld()->SpawnActor<AActor>(DestinationMarker, SpawnLocation, {}, {});
	CachedDestinationMarker->SetActorHiddenInGame(true);

	PartySubsystem = GetGameInstance()->GetSubsystem<URPartySubsystem>();

	// Create and register the Game HUD
	if (IsLocalController() && GameHUDClass)
	{
		GameHUDWidget = CreateWidget<URGameHUD>(this, GameHUDClass);
		if (auto* Ctrl = GetGameInstance()->GetSubsystem<URUILayerController>())
			Ctrl->RegisterGameHUD(GameHUDWidget, this);
		GameHUDWidget->AddToViewport();
	}

}

void ARPlayerControllerBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	GetLocationUnderCursor(CachedInteraction);
	ShowPathVisualization(PartySubsystem->GetSelectedCharacter(), CachedInteraction);
}

void ARPlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Optional: handle input like selecting skills, navigating UI, etc.
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		//Camera
		EnhancedInput->BindAction(CameraMoveAction, ETriggerEvent::Triggered, this,
		                          &ARPlayerControllerBase::MoveCamera);
		EnhancedInput->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ARPlayerControllerBase::ZoomCamera);
		EnhancedInput->BindAction(RotateCameraAction, ETriggerEvent::Triggered, this,
		                          &ARPlayerControllerBase::RotateCamera);
		EnhancedInput->BindAction(LockCameraAction, ETriggerEvent::Started, this, &ARPlayerControllerBase::LockCamera);

		//Exploration
		EnhancedInput->BindAction(CharacterMoveAction, ETriggerEvent::Completed, this,
		                          &ARPlayerControllerBase::InteractClick);

		EnhancedInput->BindAction(CharacterMoveActionHold, ETriggerEvent::Triggered, this,
		                          &ARPlayerControllerBase::InteractClickHold);
		EnhancedInput->BindAction(CharacterMoveActionHold, ETriggerEvent::Completed, this,
		                          &ARPlayerControllerBase::InteractClickCompleted);

		EnhancedInput->BindAction(ToggleWorldViewAction, ETriggerEvent::Started, this,&ARPlayerControllerBase::OnToggleWorldView);
	}
}

void ARPlayerControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	// ensure we have the right pawn type
	ControlledPawn = Cast<ARPawn>(InPawn);
	check(ControlledPawn);
}

void ARPlayerControllerBase::MoveCamera(const FInputActionValue& Value)
{
	ControlledPawn->InterruptCombatPan();
	ControlledPawn->ToggleLockCamera(false);

	const FVector2D InputVector = Value.Get<FVector2D>();

	FRotator ControlRot = ControlledPawn->GetCamera()->GetComponentRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;

	const FVector Forward = ControlRot.Vector();
	ControlledPawn->AddMovementInput(Forward, InputVector.X);

	const FVector Right = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);
	ControlledPawn->AddMovementInput(Right, InputVector.Y);
}

void ARPlayerControllerBase::ZoomCamera(const FInputActionValue& Value)
{
	const float ZoomValue = Value.Get<float>();

	ZoomLevel = FMath::Clamp(ZoomLevel - ZoomValue * ZoomSpeed, MinZoomLevel, MaxZoomLevel);

	const float TargetSpeed = FMath::Lerp(MinMoveSpeed, MaxMoveSpeed, ZoomLevel);

	ControlledPawn->SetZoomModifier(
		ZoomLevel,
		MinZoomLength,
		MaxZoomLength,
		MinPitch,
		MaxPitch,
		ZoomInterpSpeed,
		TargetSpeed
	);

	if (bFOVFollowsZoom)
	{
		const float NewFOV = FMath::Lerp(MinFOV, MaxFOV, ZoomLevel);
		if (UCameraComponent* Cam = ControlledPawn ? ControlledPawn->GetCamera() : nullptr)
		{
			Cam->SetFieldOfView(NewFOV);
		}
	}
}

void ARPlayerControllerBase::RotateCamera()
{
	ControlledPawn->InterruptCombatPan();
	FVector2D Input;
	GetInputMouseDelta(Input.X, Input.Y);
	ControlledPawn->RotateCamera(Input.X * RotationSens);
}

void ARPlayerControllerBase::LockCamera()
{
	ControlledPawn->ToggleLockCamera();
}

bool ARPlayerControllerBase::GetLocationUnderCursor(FVector& Location)
{
	// trace the visibility channel at the cursor location
	FHitResult OutHit;

	GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, OutHit);

	// if there was a blocking hit, return the hit location
	if (OutHit.bBlockingHit)
	{
		Location = OutHit.Location;
		return true;
	}

	return OutHit.bBlockingHit;
}

void ARPlayerControllerBase::InteractClick(const FInputActionValue& InputActionValue)
{
	FHitResult Hit;

	if (!GetHitResultUnderCursorByChannel(
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		Hit))
	{
		return;
	}

	ARCharacterBase* SelectedChar = PartySubsystem
		? PartySubsystem->GetSelectedCharacter()
		: nullptr;

	if (!SelectedChar)
	{
		return;
	}

	AActor* HitActor = Hit.GetActor();


	if (HitActor && HitActor->GetClass()->ImplementsInterface(URInteractable::StaticClass()))
	{
		if (SelectedChar->Interact(*HitActor))
		{
			// We handed control to the ability – no manual MoveTo here.
			if (CachedDestinationMarker)
			{
				CachedDestinationMarker->SetActorHiddenInGame(true);
			}
			return;
		}
	}

	CachedInteraction = Hit.Location;
	MoveCharacter();

	if (CachedDestinationMarker)
	{
		CachedDestinationMarker->SetActorLocation(CachedInteraction);
		CachedDestinationMarker->SetActorHiddenInGame(false);
	}
}

void ARPlayerControllerBase::InteractClickHold(const FInputActionValue& InputActionValue)
{
	if (!GetLocationUnderCursor(CachedInteraction))
	{
		return;
	}
	ControlledPawn->ToggleLockCamera(true);
	MoveCharacter();
}

void ARPlayerControllerBase::InteractClickCompleted()
{
	PartySubsystem->GetSelectedCharacter()->StopMoving();
}

void ARPlayerControllerBase::MoveCharacter()
{
	PartySubsystem->GetSelectedCharacter()->MoveToLocation(CachedInteraction);
	PartySubsystem->GetSelectedCharacter()->GetAIController()->ReceiveMoveCompleted.RemoveDynamic(
		this, &ARPlayerControllerBase::OnMoveCompleted);
	PartySubsystem->GetSelectedCharacter()->GetAIController()->ReceiveMoveCompleted.AddDynamic(
		this, &ARPlayerControllerBase::OnMoveCompleted);
}

void ARPlayerControllerBase::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	if (!CachedDestinationMarker)
	{
		return;
	}
	CachedDestinationMarker->SetActorHiddenInGame(true);

	if (ARCharacterBase* Char = Cast<ARCharacterBase>(PartySubsystem->GetSelectedCharacter()))
		Char->ClearPathPreview();

	PartySubsystem->GetSelectedCharacter()->GetAIController()->ReceiveMoveCompleted.RemoveDynamic(
		this, &ARPlayerControllerBase::OnMoveCompleted);
}

void ARPlayerControllerBase::ShowPathVisualization(ACharacter* InCharacter, const FVector& TargetLocation)
{
	ARCharacterBase* RCharacter = Cast<ARCharacterBase>(InCharacter);
	if (!RCharacter)
	{
		return;
	}

	const URTurnComponent* TurnComp = RCharacter->GetTurnComponent();
	if (!TurnComp || TurnComp->TurnState != ETurnState::InTurn)
	{
		LastPathQueryTarget = FVector(FLT_MAX);
		LastPathQueryOrigin = FVector(FLT_MAX);
		RCharacter->ClearPathPreview();
		return;
	}

	if (RCharacter->IsWalkPathFrozen())
	{
		RCharacter->UpdateWalkPathShrink();
		return;
	}

	const FVector CharOrigin = RCharacter->GetActorLocation();
	if (FVector::DistSquared(TargetLocation, LastPathQueryTarget) < PathRecomputeThresholdSq &&
		FVector::DistSquared(CharOrigin, LastPathQueryOrigin) < PathRecomputeThresholdSq)
	{
		return;
	}
	LastPathQueryTarget = TargetLocation;
	LastPathQueryOrigin = CharOrigin;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	FNavLocation ProjectedTarget;
	if (!NavSys || !NavSys->ProjectPointToNavigation(TargetLocation, ProjectedTarget))
	{
		RCharacter->ClearPathPreview();
		return;
	}

	UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(
		GetWorld(), CharOrigin, ProjectedTarget);

	if (!NavPath || !NavPath->IsValid() || NavPath->PathPoints.Num() < 2)
	{
		RCharacter->ClearPathPreview();
		return;
	}

	RCharacter->ShowPathPreview(NavPath->PathPoints, PathVisualizationConfig);
}
void ARPlayerControllerBase::OnToggleWorldView()
{
	if (URWorldStateSubsystem* WorldState = GetWorld()->GetSubsystem<URWorldStateSubsystem>())
	{
		WorldState->ToggleWorldView();
	}
}
