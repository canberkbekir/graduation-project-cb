// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Events/WorldViewEvents.h"
#include "RWorldStateSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWorldViewPreToggle,  EWorldView, FromView, EWorldView, ToView);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnWorldViewApplied,    EWorldView, NewView);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnWorldViewPostToggle, EWorldView, NewView);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnWorldViewChangedBP,  EWorldView, NewView);
DECLARE_DYNAMIC_MULTICAST_DELEGATE         (FOnWorldViewToggleCancelled);

/**
 * Single source of truth for which world view is active (Physical or Network).
 *
 * Toggle flow: SetWorldView → Pre → Apply (volume flip + events) → Post → Idle.
 * bUsePreTogglePhase / bUsePostTogglePhase enable async BP phases — BP calls
 * FinishPreToggle / FinishPostToggle when its cinematic/fade is done.
 * When both flags are false the entire cycle completes synchronously (original behavior).
 *
 * BP component on controller: bind to OnWorldViewPreToggle / OnWorldViewApplied /
 * OnWorldViewPostToggle / OnWorldViewChanged in BeginPlay.
 * Override OnPreToggle / OnPostToggle in a Blueprint subclass to gate the flow.
 *
 * C++ override: derive and override ExecutePreToggle / ExecuteApplyToggle /
 * ExecutePostToggle / CanToggleTo for full control without Blueprint.
 */
UCLASS()
class RIZZGAME_API URWorldStateSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// -----------------------------------------------------------------------
	// Config
	// -----------------------------------------------------------------------

	/** When true, OnPreToggle BP event fires and the flow waits for FinishPreToggle(). */
	UPROPERTY(EditDefaultsOnly, Category="Rizz|WorldView|Config")
	bool bUsePreTogglePhase = false;

	/** When true, OnPostToggle BP event fires and the flow waits for FinishPostToggle(). */
	UPROPERTY(EditDefaultsOnly, Category="Rizz|WorldView|Config")
	bool bUsePostTogglePhase = false;

	/** Actor tag used to find the NetworkWorld PostProcessVolume in the level. */
	UPROPERTY(EditDefaultsOnly, Category="Rizz|WorldView|Config")
	FName VolumeTag = "NetworkWorld";

	// -----------------------------------------------------------------------
	// State (read-only from BP)
	// -----------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category="Rizz|WorldView")
	EWorldView CurrentView = EWorldView::Physical;

	UPROPERTY(BlueprintReadOnly, Category="Rizz|WorldView")
	EWorldViewPhase CurrentPhase = EWorldViewPhase::Idle;

	/** Target view during an in-flight transition; equals CurrentView when Idle. */
	UPROPERTY(BlueprintReadOnly, Category="Rizz|WorldView")
	EWorldView PendingView = EWorldView::Physical;

	// -----------------------------------------------------------------------
	// Delegates — bind from BP component in BeginPlay
	// -----------------------------------------------------------------------

	/** Fired at the start of every toggle, before anything is applied. */
	UPROPERTY(BlueprintAssignable, Category="Rizz|WorldView|Events")
	FOnWorldViewPreToggle OnWorldViewPreToggle;

	/** Fired immediately after the PostProcessVolume is flipped. */
	UPROPERTY(BlueprintAssignable, Category="Rizz|WorldView|Events")
	FOnWorldViewApplied OnWorldViewApplied;

	/** Fired when the post-toggle phase begins. */
	UPROPERTY(BlueprintAssignable, Category="Rizz|WorldView|Events")
	FOnWorldViewPostToggle OnWorldViewPostToggle;

	/** Fired when the full toggle cycle completes (Idle again). */
	UPROPERTY(BlueprintAssignable, Category="Rizz|WorldView|Events")
	FOnWorldViewChangedBP OnWorldViewChanged;

	/** Fired when a pending toggle is cancelled before Apply. */
	UPROPERTY(BlueprintAssignable, Category="Rizz|WorldView|Events")
	FOnWorldViewToggleCancelled OnWorldViewToggleCancelled;

	// -----------------------------------------------------------------------
	// Blueprint Implementable Events — override in BP subclass
	// -----------------------------------------------------------------------

	/** Override to run a pre-toggle cinematic or fade. Call FinishPreToggle() when done.
	 *  Only gates the flow when bUsePreTogglePhase is true. */
	UFUNCTION(BlueprintImplementableEvent, Category="Rizz|WorldView|Events")
	void OnPreToggle(EWorldView FromView, EWorldView ToView);

	/** Cosmetic hook called immediately after the volume is flipped.
	 *  Does not gate the flow — use for extra VFX / sound. */
	UFUNCTION(BlueprintImplementableEvent, Category="Rizz|WorldView|Events")
	void OnApplyToggle(EWorldView NewView);

	/** Override to run post-toggle logic. Call FinishPostToggle() when done.
	 *  Only gates the flow when bUsePostTogglePhase is true. */
	UFUNCTION(BlueprintImplementableEvent, Category="Rizz|WorldView|Events")
	void OnPostToggle(EWorldView NewView);

	// -----------------------------------------------------------------------
	// Blueprint Callable API
	// -----------------------------------------------------------------------

	/** Main entry point. Cancels any in-flight toggle and starts a new one. */
	UFUNCTION(BlueprintCallable, Category="Rizz|WorldView")
	void SetWorldView(EWorldView NewView);

	UFUNCTION(BlueprintCallable, Category="Rizz|WorldView")
	void ToggleWorldView();

	/** Call from Blueprint when your pre-toggle sequence (cinematic, fade) is finished. */
	UFUNCTION(BlueprintCallable, Category="Rizz|WorldView")
	void FinishPreToggle();

	/** Call from Blueprint when your post-toggle sequence is finished. */
	UFUNCTION(BlueprintCallable, Category="Rizz|WorldView")
	void FinishPostToggle();

	/** Cancel a toggle that is still in the PreToggle phase (volume not yet flipped). */
	UFUNCTION(BlueprintCallable, Category="Rizz|WorldView")
	void CancelPendingToggle();

	UFUNCTION(BlueprintPure, Category="Rizz|WorldView")
	EWorldView GetCurrentView() const { return CurrentView; }

	UFUNCTION(BlueprintPure, Category="Rizz|WorldView")
	EWorldViewPhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category="Rizz|WorldView")
	bool IsInNetworkWorld() const { return CurrentView == EWorldView::Network; }

	/** True while a toggle cycle is running (PreToggle, Applying, or PostToggle). */
	UFUNCTION(BlueprintPure, Category="Rizz|WorldView")
	bool IsTransitioning() const { return CurrentPhase != EWorldViewPhase::Idle; }

	// -----------------------------------------------------------------------
	// C++ Virtual Hooks — override in derived C++ class
	// -----------------------------------------------------------------------

	/** Return false to block a toggle (e.g. disallow NetworkWorld during combat). */
	virtual bool CanToggleTo(EWorldView NewView) const { return true; }

	virtual void ExecutePreToggle(EWorldView FromView, EWorldView ToView);
	virtual void ExecuteApplyToggle(EWorldView NewView);
	virtual void ExecutePostToggle(EWorldView NewView);
};
