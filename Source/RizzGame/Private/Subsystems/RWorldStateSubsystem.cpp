// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/RWorldStateSubsystem.h"
#include "Subsystems/REventBusSubsystem.h"
#include "Engine/PostProcessVolume.h"
#include "EngineUtils.h"

void URWorldStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void URWorldStateSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void URWorldStateSubsystem::SetWorldView(EWorldView NewView)
{
	if (!CanToggleTo(NewView))
	{
		return;
	}

	if (CurrentPhase != EWorldViewPhase::Idle)
	{
		CancelPendingToggle();
	}

	if (NewView == CurrentView)
	{
		return;
	}

	PendingView  = NewView;
	CurrentPhase = EWorldViewPhase::PreToggle;

	ExecutePreToggle(CurrentView, PendingView);
}

void URWorldStateSubsystem::ToggleWorldView()
{
	SetWorldView(CurrentView == EWorldView::Physical ? EWorldView::Network : EWorldView::Physical);
}

void URWorldStateSubsystem::FinishPreToggle()
{
	if (CurrentPhase != EWorldViewPhase::PreToggle)
	{
		return;
	}

	CurrentPhase = EWorldViewPhase::Applying;
	ExecuteApplyToggle(PendingView);
}

void URWorldStateSubsystem::FinishPostToggle()
{
	if (CurrentPhase != EWorldViewPhase::PostToggle)
	{
		return;
	}

	CurrentPhase = EWorldViewPhase::Idle;
	PendingView  = CurrentView;

	OnWorldViewChanged.Broadcast(CurrentView);
}

void URWorldStateSubsystem::CancelPendingToggle()
{
	if (CurrentPhase == EWorldViewPhase::Idle)
	{
		return;
	}

	const EWorldView AbortedAt = CurrentView;
	CurrentPhase = EWorldViewPhase::Idle;
	PendingView  = CurrentView;

	OnWorldViewToggleCancelled.Broadcast();

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UREventBusSubsystem* Bus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			FWorldViewToggleCancelled Event;
			Event.AbortedAtView = AbortedAt;
			Bus->Publish(Event);
		}
	}
}

// ---------------------------------------------------------------------------
// Virtual Execute hooks
// ---------------------------------------------------------------------------

void URWorldStateSubsystem::ExecutePreToggle(EWorldView FromView, EWorldView ToView)
{
	OnWorldViewPreToggle.Broadcast(FromView, ToView);

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UREventBusSubsystem* Bus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			FWorldViewTogglePreBegin Event;
			Event.FromView = FromView;
			Event.ToView   = ToView;
			Bus->Publish(Event);
		}
	}

	OnPreToggle(FromView, ToView);

	// Auto-advance when bUsePreTogglePhase is disabled, or when nothing handles the phase.
	if (!bUsePreTogglePhase ||
		(!OnWorldViewPreToggle.IsBound() &&
		 !GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(URWorldStateSubsystem, OnPreToggle))))
	{
		FinishPreToggle();
	}
}

void URWorldStateSubsystem::ExecuteApplyToggle(EWorldView NewView)
{
	CurrentView = NewView;

	// Flip the tagged PostProcessVolume.
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			if (It->ActorHasTag(VolumeTag))
			{
				It->bEnabled = (NewView == EWorldView::Network);
				break;
			}
		}
	}

	OnWorldViewApplied.Broadcast(NewView);

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UREventBusSubsystem* Bus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			FWorldViewToggleApplied AppliedEvent;
			AppliedEvent.NewView = NewView;
			Bus->Publish(AppliedEvent);

			// Backward compatibility — BP listeners on FWorldViewChanged still receive this.
			FWorldViewChanged CompatEvent;
			CompatEvent.NewView = NewView;
			Bus->Publish(CompatEvent);
		}
	}

	OnApplyToggle(NewView);

	CurrentPhase = EWorldViewPhase::PostToggle;
	ExecutePostToggle(NewView);
}

void URWorldStateSubsystem::ExecutePostToggle(EWorldView NewView)
{
	OnWorldViewPostToggle.Broadcast(NewView);

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UREventBusSubsystem* Bus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			FWorldViewTogglePostBegin Event;
			Event.NewView = NewView;
			Bus->Publish(Event);
		}
	}

	OnPostToggle(NewView);

	// Auto-advance when bUsePostTogglePhase is disabled, or when nothing handles the phase.
	if (!bUsePostTogglePhase ||
		(!OnWorldViewPostToggle.IsBound() &&
		 !GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(URWorldStateSubsystem, OnPostToggle))))
	{
		FinishPostToggle();
	}
}
