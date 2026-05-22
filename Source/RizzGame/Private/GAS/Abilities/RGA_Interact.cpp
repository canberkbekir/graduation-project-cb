#include "GAS/Abilities/RGA_Interact.h"

#include "AIController.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Interaction/RInteractable.h"
#include "Interaction/RInteractionAreaComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemComponent.h"
#include "Components/RTurnComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogRGAInteract, Log, All);

URGA_Interact::URGA_Interact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Interact"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	DefaultInteractionRadius = 150.f;
	bRotateToFaceInteractionPoint = true;

	bHasActiveMoveRequest = false;
	PendingAreaIndex = 0;
}

bool URGA_Interact::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	const bool bSuperCan = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	UE_LOG(LogRGAInteract, Verbose,
		   TEXT("URGA_Interact::CanActivateAbility => %s (Owner=%s)"),
		   bSuperCan ? TEXT("true") : TEXT("false"),
		   ActorInfo && ActorInfo->OwnerActor.IsValid() ? *ActorInfo->OwnerActor->GetName() : TEXT("None"));

	return bSuperCan;
}

static bool IsOutTurn(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (const APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get()))
	{
		if (const URTurnComponent* TurnComp = Pawn->GetComponentByClass<URTurnComponent>())
		{
			return TurnComp->TurnState == ETurnState::OutTurn;
		}
	}
	return false;
}

bool URGA_Interact::CheckCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (IsOutTurn(ActorInfo)) return true;
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

bool URGA_Interact::CheckCooldown(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (IsOutTurn(ActorInfo)) return true;
	return Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags);
}

void URGA_Interact::ApplyCooldown(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (IsOutTurn(ActorInfo)) return;
	Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
}

void URGA_Interact::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!ActivationSetup(Handle, ActorInfo, ActivationInfo, TriggerEventData))
    {
        return;
    }

    // ActivationSetup no longer commits AP — Interact commits it here.
    // CheckCost/CheckCooldown/ApplyCooldown overrides handle the OutTurn case correctly.
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get());
    if (!Pawn)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AActor* TargetActor = nullptr;
    if (TriggerEventData && TriggerEventData->Target)
    {
        TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());
    }

    if (!TargetActor || !TargetActor->GetClass()->ImplementsInterface(URInteractable::StaticClass()))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    CurrentInteractable = TargetActor;

    TArray<URInteractionAreaComponent*> Areas;
    TargetActor->GetComponents<URInteractionAreaComponent>(Areas);
    const FVector PawnLocation = Pawn->GetActorLocation();

    Areas.Sort([&PawnLocation](const URInteractionAreaComponent& A, const URInteractionAreaComponent& B)
    {
        const float DistA = FVector::DistSquared(PawnLocation, A.GetComponentLocation());
        const float DistB = FVector::DistSquared(PawnLocation, B.GetComponentLocation());
        return DistA < DistB;
    });

    FVector ClosestLocation;
    float ClosestRadius;

    if (Areas.Num() > 0)
    {
        ClosestLocation = Areas[0]->GetInteractionLocation();
        ClosestRadius = Areas[0]->GetInteractionRadius();
    }
    else
    {
        ClosestLocation = IRInteractable::Execute_GetInteractionLocation(TargetActor);
        ClosestRadius = IRInteractable::Execute_GetInteractionRadius(TargetActor);
    }

    if (ClosestRadius <= 0.f) ClosestRadius = DefaultInteractionRadius;

    const float DistSq = FVector::DistSquaredXY(PawnLocation, ClosestLocation);
    if (DistSq <= FMath::Square(ClosestRadius))
    {
        // Defer one tick so EndAbility fires outside the GAS scope lock.
        InRangeInteractionTimer = GetWorld()->GetTimerManager().SetTimerForNextTick(
            this, &URGA_Interact::OnInRangeTimerFired);
        return;
    }

    PendingAreas.Reset();
    PendingAreas.Reserve(Areas.Num());
    for (URInteractionAreaComponent* Area : Areas)
    {
        PendingAreas.Add(Area);
    }
    PendingAreaIndex = 0;

    TryNextArea(Pawn);
}

void URGA_Interact::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	UE_LOG(LogRGAInteract, Log,
		   TEXT("URGA_Interact::EndAbility (Cancelled=%d)"),
		   bWasCancelled ? 1 : 0);

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get()))
		{
			if (AAIController* AICon = Cast<AAIController>(Pawn->GetController()))
			{
				if (UPathFollowingComponent* PFC = AICon->GetPathFollowingComponent())
				{
					PFC->OnRequestFinished.RemoveAll(this);
				}
			}
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InRangeInteractionTimer);
	}

	bHasActiveMoveRequest = false;
	CurrentInteractable.Reset();
	PendingAreas.Reset();
	PendingAreaIndex = 0;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URGA_Interact::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	UE_LOG(LogRGAInteract, Log,
		   TEXT("URGA_Interact::OnMoveCompleted Result=%d (HasActive=%d)"), (int32)Result.Code,
		   bHasActiveMoveRequest ? 1 : 0);

	if (!bHasActiveMoveRequest || RequestID != ActiveMoveRequestID)
	{
		return;
	}

	bHasActiveMoveRequest = false;

	const FGameplayAbilityActorInfo* ActorInfo = CurrentActorInfo;
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		UE_LOG(LogRGAInteract, Warning, TEXT("OnMoveCompleted: no valid ActorInfo/AvatarActor"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get());
	AAIController* AICon = Pawn ? Cast<AAIController>(Pawn->GetController()) : nullptr;

	if (AICon)
	{
		if (UPathFollowingComponent* PFC = AICon->GetPathFollowingComponent())
		{
			PFC->OnRequestFinished.RemoveAll(this);
		}
	}

	if (Result.Code == EPathFollowingResult::Success)
	{
		PerformInteraction(Pawn);
	}
	else
	{
		UE_LOG(LogRGAInteract, Warning, TEXT("Move to area failed, trying next area..."));
		TryNextArea(Pawn);
	}
}

bool URGA_Interact::IssueMoveToLocation(AAIController* AICon, APawn* Pawn, const FVector& Location, float AcceptanceRadius)
{
	UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(
		Pawn, Pawn->GetActorLocation(), Location);

	if (!Path || !Path->IsValid() || Path->PathPoints.Num() < 2)
	{
		return false;
	}

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(Location);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetAllowPartialPath(true);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);

	FNavPathSharedPtr NavPath;
	const FPathFollowingRequestResult Result = AICon->MoveTo(MoveRequest, &NavPath);

	if (Result.Code == EPathFollowingRequestResult::Failed)
	{
		return false;
	}

	// AlreadyAtGoal fires OnRequestFinished synchronously inside MoveTo, before we can subscribe.
	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		InRangeInteractionTimer = GetWorld()->GetTimerManager().SetTimerForNextTick(
			this, &URGA_Interact::OnInRangeTimerFired);
		return true;
	}

	ActiveMoveRequestID = Result.MoveId;
	bHasActiveMoveRequest = true;

	if (UPathFollowingComponent* PFC = AICon->GetPathFollowingComponent())
	{
		PFC->OnRequestFinished.AddUObject(this, &URGA_Interact::OnMoveCompleted);
	}

	return true;
}

void URGA_Interact::TryNextArea(APawn* Pawn)
{
	if (!Pawn)
	{
		UE_LOG(LogRGAInteract, Warning, TEXT("TryNextArea: Pawn is null"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	AAIController* AICon = Cast<AAIController>(Pawn->GetController());
	if (!AICon)
	{
		UE_LOG(LogRGAInteract, Warning, TEXT("TryNextArea: Pawn has no AAIController"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	while (PendingAreaIndex < PendingAreas.Num())
	{
		URInteractionAreaComponent* Area = PendingAreas[PendingAreaIndex].Get();
		++PendingAreaIndex;

		if (!Area) continue;

		const FVector TargetLocation = Area->GetInteractionLocation();
		float Radius = Area->GetInteractionRadius();
		if (Radius <= 0.f) Radius = DefaultInteractionRadius;

		if (FVector::DistSquaredXY(Pawn->GetActorLocation(), TargetLocation) <= FMath::Square(Radius))
		{
			InRangeInteractionTimer = GetWorld()->GetTimerManager().SetTimerForNextTick(
				this, &URGA_Interact::OnInRangeTimerFired);
			return;
		}

		if (IssueMoveToLocation(AICon, Pawn, TargetLocation, Radius))
		{
			UE_LOG(LogRGAInteract, Log,
				TEXT("TryNextArea: moving to area at %s (AcceptanceRadius=%.2f, remaining=%d)"),
				*TargetLocation.ToString(), Radius, PendingAreas.Num() - PendingAreaIndex);
			return;
		}

		UE_LOG(LogRGAInteract, Log,
			TEXT("TryNextArea: no reachable path to area at %s, trying next..."),
			*TargetLocation.ToString());
	}

	AActor* TargetActor = CurrentInteractable.Get();
	if (TargetActor)
	{
		const FVector TargetLocation = IRInteractable::Execute_GetInteractionLocation(TargetActor);
		float Radius = IRInteractable::Execute_GetInteractionRadius(TargetActor);
		if (Radius <= 0.f) Radius = DefaultInteractionRadius;

		if (IssueMoveToLocation(AICon, Pawn, TargetLocation, Radius))
		{
			return;
		}
	}

	UE_LOG(LogRGAInteract, Warning, TEXT("TryNextArea: all interaction areas unreachable"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URGA_Interact::OnInRangeTimerFired()
{
	if (!bIsActive) return;

	const FGameplayAbilityActorInfo* Info = CurrentActorInfo;
	if (!Info || !Info->AvatarActor.IsValid())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	PerformInteraction(Cast<APawn>(Info->AvatarActor.Get()));
}

void URGA_Interact::PerformInteraction(APawn* Pawn)
{
	AActor* Interactable = CurrentInteractable.Get();
	if (!Interactable || !Interactable->GetClass()->ImplementsInterface(URInteractable::StaticClass()))
	{
		UE_LOG(LogRGAInteract, Warning, TEXT("PerformInteraction: no valid interactable"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (bRotateToFaceInteractionPoint && Pawn)
	{
		const FVector TargetLocation = IRInteractable::Execute_GetInteractionLocation(Interactable);
		const FVector ToTarget       = (TargetLocation - Pawn->GetActorLocation()).GetSafeNormal2D();

		if (!ToTarget.IsNearlyZero())
		{
			Pawn->SetActorRotation(ToTarget.Rotation());
		}
	}

	UE_LOG(LogRGAInteract, Log,
		   TEXT("PerformInteraction: calling IRInteractable::Interact on %s"),
		   *Interactable->GetName());

	// EndAbility before Execute_Interact so bIsActive is cleared before Blueprint re-activates.
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

	IRInteractable::Execute_Interact(Interactable, Pawn);
}

float URGA_Interact::GetEffectiveRange() const
{
	return MAX_FLT;
}
