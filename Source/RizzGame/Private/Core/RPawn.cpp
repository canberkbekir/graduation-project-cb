// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RPawn.h"
#include "Core/RCharacterBase.h"
#include "Subsystems/RPartySubsystem.h"
#include "Subsystems/RCameraWallSubsystem.h"
#include "Subsystems/RCombatManagerSubsystem.h"
#include "Subsystems/REventBusSubsystem.h"
#include "Events/CombatEvents.h"

// Sets default values
ARPawn::ARPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// create the root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// create the SpringArm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);

	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bDoCollisionTest = false;


	// create the camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	Camera->ProjectionMode = ECameraProjectionMode::Perspective;

	// create the movement component
	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Floating Pawn Movement"));

	FloatingPawnMovement->bConstrainToPlane = true;
	FloatingPawnMovement->SetPlaneConstraintNormal(FVector::UpVector);
}

void ARPawn::BeginPlay()
{
	Super::BeginPlay();
	PartySubsystem = GetGameInstance()->GetSubsystem<URPartySubsystem>();
	CameraWallSubsystem = GetWorld()->GetSubsystem<URCameraWallSubsystem>();
	PreviousLocation = GetActorLocation();
	PreviousCameraWorldLocation = FVector::ZeroVector;

	if (UREventBusSubsystem* Bus = GetGameInstance()->GetSubsystem<UREventBusSubsystem>())
	{
		Bus->Subscribe<FCombatStarted>(this, &ARPawn::HandleCombatStarted);
		Bus->Subscribe<FCombatEnded>(this, &ARPawn::HandleCombatEnded);
		Bus->Subscribe<FTurnStarted>(this, &ARPawn::HandleTurnStarted);
	}
}

void ARPawn::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	SpringArm->TargetArmLength = FMath::FInterpTo(
		SpringArm->TargetArmLength,
		m_TargetZoomLength,
		DeltaTime,
		m_ZoomInterpSpeed
	);

	const FRotator CurrentRot = SpringArm->GetRelativeRotation();
	const FRotator TargetRot(m_TargetZoomPitch, CurrentRot.Yaw, CurrentRot.Roll);

	SpringArm->SetRelativeRotation(FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, m_ZoomInterpSpeed));

	if (PartySubsystem && PartySubsystem->GetSelectedCharacter())
	{
		const FVector CharLocation = PartySubsystem->GetSelectedCharacter()->GetActorLocation();
		if (m_bInCombat && m_bCombatCameraEnabled)
		{
			// One-shot pan toward the active combatant. Stops when arrived or when player gives input.
			const FVector PanTarget = PanTargetActor.IsValid()
				? PanTargetActor->GetActorLocation()
				: CharLocation;
			const FVector CurrentPos = GetActorLocation();
			const FVector NewPos = FMath::VInterpTo(CurrentPos, PanTarget, DeltaTime, CombatPanSpeed);
			SetActorLocation(NewPos);

			// Auto-stop when close enough (XY distance only — Z is always synced below)
			if (FVector::Dist2D(NewPos, PanTarget) < CombatPanArrivalThreshold)
			{
				m_bCombatCameraEnabled = false;
			}
		}
		else if (m_bLockCamera)
		{
			SetActorLocation(CharLocation);
		}
		else
		{
			FVector Pos = GetActorLocation();
			Pos.Z = CharLocation.Z;
			SetActorLocation(Pos);
		}
	}

	// Clamp camera position to active boundary walls (must run after lock-camera/Z-sync override)
	if (CameraWallSubsystem)
	{
		const FVector CurrentPos = GetActorLocation();
		const FVector ClampedPos = CameraWallSubsystem->ClampPositionToWalls(
			PreviousLocation, CurrentPos);
		if (!ClampedPos.Equals(CurrentPos))
		{
			SetActorLocation(ClampedPos);
		}

		// Also clamp the camera actor (end of spring arm) against walls with bBlockCameraActor.
		// Compute camera world position from current pawn position and spring arm state.
		const FVector PawnPos = GetActorLocation();
		const FRotator ArmWorldRot = SpringArm->GetComponentRotation();
		const FVector ArmForward = ArmWorldRot.Vector();
		const FVector CameraWorldPos = PawnPos - ArmForward * SpringArm->TargetArmLength;

		// Initialize previous camera location on first frame
		if (PreviousCameraWorldLocation.IsZero())
		{
			PreviousCameraWorldLocation = CameraWorldPos;
		}

		const FVector ClampedCameraPos = CameraWallSubsystem->ClampCameraActorToWalls(
			PreviousCameraWorldLocation, CameraWorldPos);
		if (!ClampedCameraPos.Equals(CameraWorldPos))
		{
			// Push the pawn by the same XY delta so the camera sits at the wall
			const FVector Delta(ClampedCameraPos.X - CameraWorldPos.X,
				ClampedCameraPos.Y - CameraWorldPos.Y, 0.0f);
			SetActorLocation(PawnPos + Delta);
		}

		// Store camera location for next frame's crossing check
		const FVector FinalPawnPos = GetActorLocation();
		PreviousCameraWorldLocation = FinalPawnPos - ArmForward * SpringArm->TargetArmLength;
	}
	PreviousLocation = GetActorLocation();
}


void ARPawn::SetZoomModifier(const float ZoomLevel, const float MinZoomLength, const float MaxZoomLength,
                             const float MinPitch, const float MaxPitch, const float ZoomSpeed, const float MoveSpeed)
{
	m_TargetZoomLength = FMath::Lerp(MinZoomLength, MaxZoomLength, ZoomLevel);
	m_TargetZoomPitch = FMath::Lerp(MinPitch, MaxPitch, ZoomLevel);
	m_ZoomInterpSpeed = ZoomSpeed;
	FloatingPawnMovement->MaxSpeed = MoveSpeed;
}

void ARPawn::RotateCamera(const float MouseDelta)
{
	FRotator NewRot = SpringArm->GetRelativeRotation();
	NewRot.Yaw += MouseDelta;
	SpringArm->SetRelativeRotation(NewRot);
}

void ARPawn::ToggleLockCamera(TOptional<bool> bLocked)
{
	if (bLocked.IsSet())
	{
		m_bLockCamera = bLocked.GetValue();
		return;
	}
	m_bLockCamera = !m_bLockCamera;
	UE_LOG(LogHAL, Log, TEXT("Camera lock toggled"));
}

void ARPawn::InterruptCombatPan()
{
	if (m_bInCombat && m_bCombatCameraEnabled)
	{
		m_bCombatCameraEnabled = false;
	}
}

void ARPawn::HandleCombatStarted(const FCombatStarted& Event)
{
	m_bPreCombatLockState = m_bLockCamera;
	m_bInCombat = true;
	m_bCombatCameraEnabled = true; // Pan to first combatant
}

void ARPawn::HandleCombatEnded(const FCombatEnded& Event)
{
	m_bInCombat = false;
	m_bCombatCameraEnabled = false;
	m_bLockCamera = m_bPreCombatLockState;
}

void ARPawn::HandleTurnStarted(const FTurnStarted& Event)
{
	if (!m_bInCombat)
	{
		return;
	}

	URCombatManagerSubsystem* CombatMgr = GetGameInstance()->GetSubsystem<URCombatManagerSubsystem>();
	if (CombatMgr && CombatMgr->Combatants.Num() > 0)
	{
		const uint8 Index = CombatMgr->CurrentCombatantIndex;
		if (Index < static_cast<uint8>(CombatMgr->Combatants.Num()))
		{
			if (URTurnComponent* ActiveComp = CombatMgr->Combatants[Index])
			{
				PanTargetActor = ActiveComp->GetOwner();
			}
		}
	}

	m_bCombatCameraEnabled = true;
}
