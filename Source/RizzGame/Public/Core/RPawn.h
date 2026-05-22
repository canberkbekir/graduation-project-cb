// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Subsystems/RPartySubsystem.h"
#include "RPawn.generated.h"

class URCameraWallSubsystem;
struct FCombatStarted;
struct FCombatEnded;

UCLASS()
class RIZZGAME_API ARPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ARPawn();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	/** Returns the camera component */
	UCameraComponent* GetCamera() const { return Camera; }

	/** Sets the camera zoom modifier value */
	void SetZoomModifier(float ZoomLevel, float MinZoomLength, float MaxZoomLength, float MinPitch, float MaxPitch,
	                     float ZoomSpeed, float MoveSpeed);
	/**Rotates the camera*/
	void RotateCamera(float MouseDelta);
	/**
 * Toggles the camera lock state focused on the specified character.
 * @param bLocked - Set Camera Lock
 */
	void ToggleLockCamera(TOptional<bool> bLocked = TOptional<bool>());

	/** Called by the controller on any camera input to cancel an in-progress combat pan. */
	void InterruptCombatPan();

protected:
	/** Spring Arm */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;
	/** Camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;
	/** Movement Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFloatingPawnMovement> FloatingPawnMovement;

	/** Speed at which the camera pans to the active combatant during combat. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Combat")
	float CombatPanSpeed = 5.f;

	/** Enable or disable the automatic combat camera follow. */
	UFUNCTION(BlueprintCallable, Category = "Camera|Combat")
	void SetCombatCameraEnabled(bool bEnabled) { m_bCombatCameraEnabled = bEnabled; }

	/** Returns true if the combat camera follow is enabled. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Camera|Combat")
	bool IsCombatCameraEnabled() const { return m_bCombatCameraEnabled; }

	/** Distance (XY) at which the combat pan is considered complete and stops automatically. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Combat")
	float CombatPanArrivalThreshold = 80.f;

private:
	float m_ZoomInterpSpeed;
	float m_TargetZoomLength;
	float m_TargetZoomPitch;
	bool m_bLockCamera = false;
	bool m_bPreCombatLockState = false;
	bool m_bInCombat = false;
	bool m_bCombatCameraEnabled = true;

	UPROPERTY()
	TObjectPtr<URPartySubsystem> PartySubsystem;

	UPROPERTY()
	TObjectPtr<URCameraWallSubsystem> CameraWallSubsystem;

	FVector PreviousLocation;
	FVector PreviousCameraWorldLocation;

	/** The actor the camera is currently panning toward (set on each turn start). */
	TWeakObjectPtr<AActor> PanTargetActor;

	void HandleCombatStarted(const FCombatStarted& Event);
	void HandleCombatEnded(const FCombatEnded& Event);
	void HandleTurnStarted(const struct FTurnStarted& Event);
};
