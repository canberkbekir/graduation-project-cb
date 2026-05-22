#pragma once

#include "CoreMinimal.h"
#include "Core/RPawn.h"
#include "Subsystems/RPartySubsystem.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"
#include "Settings/RPathVisualizationConfig.h"
#include "UI/Common/RGameHUD.h"
#include "RPlayerControllerBase.generated.h"


namespace EPathFollowingResult
{
	enum Type : int;
}

/**
 * Base player controller — handles camera, HUD, and exploration movement.
 * Always-on: BaseMappingContext (camera + world-view toggle).
 * Exploration-only: ExplorationMappingContext (click-to-move).
 *
 * ARPlayerController extends this and adds CombatMappingContext + ClickResolver.
 */
UCLASS()
class RIZZGAME_API ARPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	ARPlayerControllerBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	// === Camera ===
	void MoveCamera(const FInputActionValue& Value);
	void ZoomCamera(const FInputActionValue& Value);
	void RotateCamera();
	void LockCamera();

	// === World Interaction ===
	bool GetLocationUnderCursor(FVector& Location);
	void InteractClick(const FInputActionValue& InputActionValue);
	void InteractClickHold(const FInputActionValue& InputActionValue);
	void InteractClickCompleted();
	void MoveCharacter();

	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);
	void ShowPathVisualization(ACharacter* InCharacter, const FVector& TargetLocation);
	void OnToggleWorldView();

	// === Properties ===

	UPROPERTY()
	TObjectPtr<ARPawn> ControlledPawn;

	UPROPERTY()
	TObjectPtr<URPartySubsystem> PartySubsystem;

	/** Always-active context: camera controls + world-view toggle. */
	UPROPERTY(EditDefaultsOnly, Category="R|Input")
	TObjectPtr<UInputMappingContext> BaseMappingContext;

	/** Active during exploration only. Removed when combat starts, restored when it ends. */
	UPROPERTY(EditDefaultsOnly, Category="R|Input")
	TObjectPtr<UInputMappingContext> ExplorationMappingContext;

	// --- Base actions (always active) ---
	UPROPERTY(EditDefaultsOnly, Category="R|Input|Camera")
	TObjectPtr<UInputAction> CameraMoveAction;

	UPROPERTY(EditDefaultsOnly, Category="R|Input|Camera")
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(EditDefaultsOnly, Category="R|Input|Camera")
	TObjectPtr<UInputAction> RotateCameraAction;

	UPROPERTY(EditDefaultsOnly, Category="R|Input|Camera")
	TObjectPtr<UInputAction> LockCameraAction;

	UPROPERTY(EditDefaultsOnly, Category="R|Input|Base")
	TObjectPtr<UInputAction> ToggleWorldViewAction;

	// --- Exploration actions ---
	UPROPERTY(EditDefaultsOnly, Category="R|Input|Exploration")
	TObjectPtr<UInputAction> CharacterMoveAction;

	UPROPERTY(EditDefaultsOnly, Category="R|Input|Exploration")
	TObjectPtr<UInputAction> CharacterMoveActionHold;

	// --- Path visualization ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "R|PathVisualization")
	TObjectPtr<URPathVisualizationConfig> PathVisualizationConfig;

	// --- Destination marker ---
	UPROPERTY(EditDefaultsOnly, Category="R|DestinationMarker")
	TSubclassOf<AActor> DestinationMarker;

	UPROPERTY()
	TObjectPtr<AActor> CachedDestinationMarker;

	FVector CachedInteraction;

	// === HUD ===
	UPROPERTY(EditDefaultsOnly, Category="R|UI")
	TSubclassOf<URGameHUD> GameHUDClass;

	UPROPERTY()
	TObjectPtr<URGameHUD> GameHUDWidget;

	// === Camera tuning ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="R|Camera|Zoom")
	float MinZoomLength = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="R|Camera|Zoom")
	float MaxZoomLength = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="R|Camera|Zoom")
	float MinPitch = -10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="R|Camera|Zoom")
	float MaxPitch = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="R|Camera|Zoom")
	float ZoomSpeed = 0.04f;

	UPROPERTY(EditAnywhere, Category="R|Camera|Zoom")
	float ZoomInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="R|Camera|FOV")
	bool bFOVFollowsZoom = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="R|Camera|FOV", meta=(ClampMin="5.0", ClampMax="170.0"))
	float MinFOV = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="R|Camera|FOV", meta=(ClampMin="5.0", ClampMax="170.0"))
	float MaxFOV = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="R|Camera|Rotation")
	float RotationSens = 3.5f;

	UPROPERTY(EditDefaultsOnly, Category="R|Camera|FloatingPawnMovement")
	float MaxMoveSpeed = 1200.0f;

	UPROPERTY(EditDefaultsOnly, Category="R|Camera|FloatingPawnMovement")
	float MinMoveSpeed = 400.0f;

private:
	float ZoomLevel = 1.f;
	static constexpr float MinZoomLevel = 0.0f;
	static constexpr float MaxZoomLevel = 1.0f;

	FVector LastPathQueryTarget = FVector(FLT_MAX);
	FVector LastPathQueryOrigin = FVector(FLT_MAX);
	static constexpr float PathRecomputeThresholdSq = 100.f;
};
