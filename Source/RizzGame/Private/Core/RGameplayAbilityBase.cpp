// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RGameplayAbilityBase.h"

#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "DrawDebugHelpers.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Core/RCharacterBase.h"
#include "Core/RPlayerController.h"
#include "Components/RTurnComponent.h"
#include "GameFramework/Character.h"
#include "GAS/Attributes/RCharacterCoreAttributeSet.h"
#include "GAS/Attributes/RCharacterDebuffAttributeSet.h"
#include "GAS/Effects/RStatusGameplayEffect.h"
#include "Components/RTurnComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GAS/Attributes/RCharacterTurnAttributeSet.h"
#include "Navigation/PathFollowingComponent.h"
#include "Subsystems/DiceSubsystem.h"
#include "Subsystems/REventBusSubsystem.h"
#include "UI/Combat/RActionBarWidget.h"

URGameplayAbilityBase::URGameplayAbilityBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool URGameplayAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo,
                                               const FGameplayTagContainer* SourceTags,
                                               const FGameplayTagContainer* TargetTags,
                                               FGameplayTagContainer* OptionalRelevantTags) const
{
	UE_LOG(LogHAL, Log, TEXT("Can Activate [bIsActive=%d]"), bIsActive ? 1 : 0);
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags))
	{
		UE_LOG(LogHAL, Warning, TEXT("Can Activate FAILED: Super (bIsActive/tags/cooldown/cost)"));
		return false;
	}

	const ARCharacterBase* Character = Cast<ARCharacterBase>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		UE_LOG(LogHAL, Warning, TEXT("Can Activate FAILED: not ARCharacterBase"));
		return false;
	}

	const URTurnComponent* TurnComponent = Character->GetComponentByClass<URTurnComponent>();
	if (!TurnComponent)
	{
		UE_LOG(LogHAL, Warning, TEXT("Can Activate FAILED: no TurnComponent"));
		return false;
	}

	const int32 StateInt = (int32)TurnComponent->TurnState;
	UE_LOG(LogHAL, Log, TEXT("Can Activate: TurnState=%d (0=OutTurn,1=InTurn,2=Waiting)"), StateInt);

	if (TurnComponent->TurnState == ETurnState::InTurn)
	{
		const UAbilitySystemComponent* Asc = Character->GetAbilitySystemComponent();
		if (!Asc)
		{
			UE_LOG(LogHAL, Warning, TEXT("Can Activate FAILED: no ASC"));
			return false;
		}

		const URCharacterTurnAttributeSet* Attributes = Asc->GetSet<URCharacterTurnAttributeSet>();
		if (!Attributes)
		{
			UE_LOG(LogHAL, Warning, TEXT("Can Activate FAILED: no Attributes"));
			return false;
		}

		const float AP = Attributes->GetActionPoints();
		UE_LOG(LogHAL, Log, TEXT("Can Activate: InTurn AP=%.1f -> %s"), AP, AP > 0 ? TEXT("true") : TEXT("false"));
		return AP > 0;
	}

	if (TurnComponent->TurnState == ETurnState::OutTurn)
	{
		UE_LOG(LogHAL, Log, TEXT("Can Activate: OutTurn -> true"));
		return true;
	}

	UE_LOG(LogHAL, Warning, TEXT("Can Activate FAILED: TurnState=Waiting"));
	return false;
}

bool URGameplayAbilityBase::ActivationSetup(const FGameplayAbilitySpecHandle& Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo,
                                             const FGameplayAbilityActivationInfo& ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
	StopSelectionMontages();
	UGameplayAbility::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UE_LOG(LogHAL, Log, TEXT("%s: URGameplayAbilityBase::ActivationSetup"), *ActorInfo->AvatarActor->GetName());
	ensure(this);
	return true;
}

void URGameplayAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo,
                                            const FGameplayAbilityActivationInfo ActivationInfo,
                                            const FGameplayEventData* TriggerEventData)
{
	if (!ActivationSetup(Handle, ActorInfo, ActivationInfo, TriggerEventData))
	{
		return;
	}

	if (TargetingType != ETargetingType::SelfCast)
	{
		if (const ARPlayerController* PC = Cast<ARPlayerController>(GetWorld()->GetFirstPlayerController()))
		{
			if (const URActionBarWidget* ActionBar = PC->GetActionBarWidget())
			{
				HitResult = ActionBar->GetPendingTargetHit();
			}
		}
	}

	switch (TargetingType)
	{
	case ETargetingType::SelfCast:
		CommitAbilityExecution();
		return;

	case ETargetingType::SingleActor:
		{
			if (CurrentEventData.Target && Cast<ARCharacterBase>(CurrentEventData.Target))
			{
				HandleTargetedActivation(Handle, ActorInfo, ActivationInfo, CurrentEventData.Target->GetActorLocation());	
				return;
			}
			ARCharacterBase* TargetActor = Cast<ARCharacterBase>(HitResult.GetActor());
			if (!TargetActor)
			{
				EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
				return;
			}
			HandleTargetedActivation(Handle, ActorInfo, ActivationInfo, TargetActor->GetActorLocation());
			return;
		}

	case ETargetingType::AOE:
		{
			if (!HitResult.bBlockingHit)
			{
				EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
				return;
			}
			HandleTargetedActivation(Handle, ActorInfo, ActivationInfo, HitResult.Location);
			return;
		}
	}
}

void URGameplayAbilityBase::HandleTargetedActivation(
	const FGameplayAbilitySpecHandle& Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo& ActivationInfo,
	const FVector& TargetPoint)
{
	ARCharacterBase* Self = Cast<ARCharacterBase>(ActorInfo->AvatarActor.Get());
	if (!Self)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	const FVector SelfLocation = Self->GetActorLocation();

	// In range or no range restriction — execute immediately.
	const float Dist = FVector::Dist(SelfLocation, TargetPoint);
	UE_LOG(LogHAL, Log, TEXT("[Targeting] Dist=%.1f Range=%.1f -> %s"),
		Dist, Range, (Range <= 0.f || Dist <= Range) ? TEXT("IN RANGE") : TEXT("OUT OF RANGE"));

	if (Range <= 0.f || Dist <= Range)
	{
		CommitAbilityExecution();
		return;
	}

	// Out of range.
	if (!bAutoApproachToRange)
	{
		UE_LOG(LogHAL, Warning, TEXT("[Targeting] Out of range and AutoApproach=false — cancelling"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector Approach = ComputeApproachLocation(SelfLocation, TargetPoint);
	if (Approach.Equals(TargetPoint, 1.f))
	{
		// ComputeApproachLocation could not project a valid navmesh point.
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float PathLength = GetNavPathLength(this, SelfLocation, Approach);
	if (PathLength < 0.f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* Asc = ActorInfo->AbilitySystemComponent.Get();
	const URCharacterTurnAttributeSet* Attributes = Asc ? Asc->GetSet<URCharacterTurnAttributeSet>() : nullptr;
	const float WalkBudget = Attributes ? Attributes->GetWalkDistance() : 0.f;
	if (WalkBudget <= 0.f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAIController* AICon = Cast<AAIController>(Self->GetController());
	if (!AICon)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	StartApproachWalk(Handle, ActorInfo, ActivationInfo, AICon, Asc, SelfLocation, Approach, PathLength, WalkBudget);
}

void URGameplayAbilityBase::StartApproachWalk(
	const FGameplayAbilitySpecHandle& Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo& ActivationInfo,
	AAIController* AICon,
	UAbilitySystemComponent* Asc,
	const FVector& SelfLocation,
	const FVector& ApproachGoal,
	float PathLength,
	float WalkBudget)
{
	FVector Goal;
	float DistanceToConsume;

	if (PathLength <= WalkBudget)
	{
		// Full walk — can reach the approach point within budget.
		Goal = ApproachGoal;
		DistanceToConsume = PathLength;
		UE_LOG(LogHAL, Log, TEXT("[ApproachWalk] FULL walk — PathLength=%.1f WalkBudget=%.1f"), PathLength, WalkBudget);
	}
	else
	{
		// Partial walk — only walk as far as WalkBudget allows.
		Goal = ComputeWaypointAlongPath(this, SelfLocation, ApproachGoal, WalkBudget);
		DistanceToConsume = WalkBudget;
		UE_LOG(LogHAL, Warning, TEXT("[ApproachWalk] PARTIAL walk — PathLength=%.1f > WalkBudget=%.1f — AP will NOT be spent if still out of range"), PathLength, WalkBudget);
	}

	UE_LOG(LogHAL, Log, TEXT("[Walk] Approach — distance: %.1f | budget unchanged"), DistanceToConsume);

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(Goal);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetAllowPartialPath(true);
	MoveRequest.SetAcceptanceRadius(Range * (1.f - RangeApproachMargin));

	FNavPathSharedPtr NavPath;
	const FPathFollowingRequestResult Result = AICon->MoveTo(MoveRequest, &NavPath);

	if (Result.Code == EPathFollowingRequestResult::Failed)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// MoveTo may have triggered a stale OnApproachWalkFinished synchronously (aborting an old move),
	// which called EndAbility on this instance before we get here. Stop here so we don't register
	// a new delegate that would become the next stale delegate.
	if (!IsActive())
	{
		return;
	}

	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		// MoveTo fires OnRequestFinished synchronously before we can subscribe — defer one tick.
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &URGameplayAbilityBase::ExecuteAbilityAfterTick);
		return;
	}

	bHasPendingApproachWalk = true;
	PendingApproachMoveID = Result.MoveId;
	PendingApproachPathComp = AICon->GetPathFollowingComponent();
	if (PendingApproachPathComp.IsValid())
	{
		PendingApproachPathComp->OnRequestFinished.AddUObject(this, &URGameplayAbilityBase::OnApproachWalkFinished);
	}
}

FVector URGameplayAbilityBase::ComputeWaypointAlongPath(const UObject* WorldContext, const FVector& Start, const FVector& End, float BudgetDistance)
{
	UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(const_cast<UObject*>(WorldContext), Start, End);
	if (!Path || !Path->IsValid() || Path->PathPoints.Num() < 2)
	{
		return End;
	}

	float Accumulated = 0.f;
	for (int32 i = 1; i < Path->PathPoints.Num(); ++i)
	{
		const FVector Prev = Path->PathPoints[i - 1];
		const FVector Curr = Path->PathPoints[i];
		const float SegLen = FVector::Dist(Prev, Curr);

		if (Accumulated + SegLen >= BudgetDistance)
		{
			const float Remaining = BudgetDistance - Accumulated;
			return Prev + (Curr - Prev).GetSafeNormal() * Remaining;
		}
		Accumulated += SegLen;
	}

	return Path->PathPoints.Last();
}

void URGameplayAbilityBase::OnApproachWalkFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (!bHasPendingApproachWalk || RequestID != PendingApproachMoveID)
	{
		return;
	}

	bHasPendingApproachWalk = false;
	if (PendingApproachPathComp.IsValid())
	{
		PendingApproachPathComp->OnRequestFinished.RemoveAll(this);
	}

	UE_LOG(LogHAL, Log, TEXT("OnApproachWalkFinished: ResultCode=%d"), static_cast<int32>(Result.Code));

	// Explicitly cancelled — abort without spending AP.
	if (Result.Code == EPathFollowingResult::Aborted)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// For Success and non-Aborted results (e.g. Blocked from partial path): verify actual distance.
	if (CurrentActorInfo && Range > 0.f)
	{
		const ARCharacterBase* Self = Cast<ARCharacterBase>(CurrentActorInfo->AvatarActor.Get());
		const AActor* Target = HitResult.GetActor();
		const FVector TargetPoint = (TargetingType == ETargetingType::SingleActor && Target)
			? Target->GetActorLocation()
			: HitResult.Location;

		if (Self)
		{
			const float Dist = FVector::Dist(Self->GetActorLocation(), TargetPoint);
			const float EffectiveRange = Range + Range * (1.f - RangeApproachMargin);
			UE_LOG(LogHAL, Log, TEXT("OnApproachWalkFinished: Dist=%.1f Range=%.1f"), Dist, Range);
			if (Dist <= EffectiveRange)
			{
				CommitAbilityExecution();
				return;
			}
			UE_LOG(LogHAL, Warning, TEXT("OnApproachWalkFinished: Still out of range after walk — cancelling"));
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
			return;
		}
	}

	if (Result.IsSuccess())
	{
		CommitAbilityExecution();
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

void URGameplayAbilityBase::ExecuteAbilityAfterTick()
{
	CommitAbilityExecution();
}

void URGameplayAbilityBase::CommitAbilityExecution_Implementation()
{
	const UAbilitySystemComponent* Asc = CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr;
	const URCharacterTurnAttributeSet* Attrs = Asc ? Asc->GetSet<URCharacterTurnAttributeSet>() : nullptr;
	const float APBefore = Attrs ? Attrs->GetActionPoints() : -1.f;

	// Commit AP cost now that execution is confirmed (character is in range).
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		UE_LOG(LogHAL, Warning, TEXT("[AP] CommitAbility FAILED (AP=%.0f) — ability cancelled without spending AP"), APBefore);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const float APAfter = Attrs ? Attrs->GetActionPoints() : -1.f;
	UE_LOG(LogHAL, Log, TEXT("[AP] %s — cost: %.0f | remaining: %.0f"),
		*GetName(), APBefore - APAfter, APAfter);
	// Default: no-op execution. Subclasses override to play AbilityExecuteMontage, apply GameplayEffects, etc.
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

float URGameplayAbilityBase::GetNavPathLength(UObject* WorldContextObject, const FVector& Start, const FVector& End)
{
	UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(WorldContextObject, Start, End);
	if (!Path || !Path->IsValid() || Path->PathPoints.Num() < 2)
	{
		return -1.f;
	}
	return Path->GetPathLength();
}

FVector URGameplayAbilityBase::ComputeApproachLocation_Implementation(
	const FVector& SelfLocation, const FVector& TargetLocation) const
{
	UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!Nav)
	{
		return TargetLocation;
	}

	// Projected point must land within this distance from target so that, after the
	// MoveTo acceptance radius (Range * (1 - Margin)), the character stops within Range.
	const float MaxApproachDist = Range * RangeApproachMargin;
	const FVector Direction = (SelfLocation - TargetLocation).GetSafeNormal();

	// Decrease the candidate distance until the navmesh projection actually lands within range.
	for (float Fraction = RangeApproachMargin; Fraction >= 0.2f; Fraction -= 0.1f)
	{
		FNavLocation Out;
		if (Nav->ProjectPointToNavigation(TargetLocation + Direction * (Range * Fraction), Out)
			&& FVector::Dist(Out.Location, TargetLocation) <= MaxApproachDist)
		{
			return Out.Location;
		}
	}

	// 8-direction sweep with the same iterative distance reduction.
	for (float Fraction = RangeApproachMargin; Fraction >= 0.2f; Fraction -= 0.1f)
	{
		const float Reach = Range * Fraction;
		for (int32 i = 0; i < 8; ++i)
		{
			const float Angle = FMath::DegreesToRadians(i * 45.f);
			const FVector Candidate = TargetLocation + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Reach;
			FNavLocation Out;
			if (Nav->ProjectPointToNavigation(Candidate, Out)
				&& FVector::Dist(Out.Location, TargetLocation) <= MaxApproachDist)
			{
				return Out.Location;
			}
		}
	}

	return TargetLocation; // Signal to caller that no valid approach point was found.
}

FAbilityIndicatorConfig URGameplayAbilityBase::GetIndicatorConfig_Implementation() const
{
	FAbilityIndicatorConfig Config;
	Config.TargetingType = TargetingType;
	Config.Range = Range;
	Config.AOERadius = AOERadius;
	return Config;
}

void URGameplayAbilityBase::DrawDebugIndicators(const FVector& SelfLocation, const FVector& CursorLocation) const
{
	if (!bShowDebugIndicators || TargetingType == ETargetingType::SelfCast)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Range circle around the character (yellow).
	if (Range > 0.f)
	{
		DrawDebugCircle(
			World,
			SelfLocation,
			Range,
			64,
			FColor::Yellow,
			false,
			-1.f,
			0,
			2.f,
			FVector::RightVector,
			FVector::ForwardVector);
	}

	// AOE radius at cursor (red).
	if (TargetingType == ETargetingType::AOE && AOERadius > 0.f)
	{
		DrawDebugCircle(
			World,
			CursorLocation,
			AOERadius,
			64,
			FColor::Red,
			false,
			-1.f,
			0,
			2.f,
			FVector::RightVector,
			FVector::ForwardVector);
	}
}

void URGameplayAbilityBase::ApplyCost(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo,
                                      const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const ARCharacterBase* Character = Cast<ARCharacterBase>(ActorInfo->AvatarActor.Get());
	if (!Character || !IsCharacterInTurn(Character))
	{
		return;
	}
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

void URGameplayAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo,
                                       const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                                       bool bWasCancelled)
{
	// Clean up any pending approach-walk delegate so it doesn't fire on a dead/restarted ability.
	if (bHasPendingApproachWalk)
	{
		bHasPendingApproachWalk = false;
		if (PendingApproachPathComp.IsValid())
		{
			PendingApproachPathComp->OnRequestFinished.RemoveAll(this);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	UE_LOG(LogHAL, Log, TEXT("%s: URGameplayAbilityBase::EndAbility"), *ActorInfo->AvatarActor->GetName());
}

bool URGameplayAbilityBase::GetHitResultUnderCursor(FHitResult& OutHitResult) const
{
	if (const APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		return PC->GetHitResultUnderCursorByChannel(
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			true,
			OutHitResult
		);
	}

	OutHitResult = FHitResult();
	return false;
}

FORCEINLINE bool URGameplayAbilityBase::IsCharacterInTurn(const ACharacter* Character) const
{
	if (!Character)
	{
		return false;
	}
	const URTurnComponent* TurnComponent = Character->GetComponentByClass<URTurnComponent>();
	if (!TurnComponent)
	{
		return false;
	}
	return TurnComponent->TurnState == ETurnState::InTurn;
}

// ────────────── Selection Lifecycle ──────────────

void URGameplayAbilityBase::OnAbilitySelectedFromBar()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		return;
	}

	BP_OnAbilitySelectedFromBar();
	ShowIndicator(Character);

	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	if (AbilitySelectMontage)
	{
		AnimInstance->Montage_Play(AbilitySelectMontage);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &URGameplayAbilityBase::OnSelectMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, AbilitySelectMontage);
	}
	else if (AfterAbilitySelectMontage)
	{
		// No select montage — play the hold/idle montage directly.
		AnimInstance->Montage_Play(AfterAbilitySelectMontage);
	}
}

void URGameplayAbilityBase::OnAbilityDeselectedFromBar(bool bIsCancelled)
{
	// Only abort an in-flight approach walk when the user explicitly cancelled.
	if (bIsCancelled && bHasPendingApproachWalk)
	{
		const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
		if (ActorInfo)
		{
			if (ACharacter* Char = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
			{
				if (AAIController* AICon = Cast<AAIController>(Char->GetController()))
				{
					AICon->StopMovement();
				}
			}
		}
	}

	StopSelectionMontages();
	HideIndicator();
	BP_OnAbilityDeselectedFromBar();
}

void URGameplayAbilityBase::OnSelectMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted || !AfterAbilitySelectMontage)
	{
		return;
	}

	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		return;
	}

	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	AnimInstance->Montage_Play(AfterAbilitySelectMontage);
}

void URGameplayAbilityBase::StopSelectionMontages()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		return;
	}

	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	if (AbilitySelectMontage && AnimInstance->Montage_IsPlaying(AbilitySelectMontage))
	{
		AnimInstance->Montage_Stop(0.25f, AbilitySelectMontage);
	}

	if (AfterAbilitySelectMontage && AnimInstance->Montage_IsPlaying(AfterAbilitySelectMontage))
	{
		AnimInstance->Montage_Stop(0.25f, AfterAbilitySelectMontage);
	}
}

FDiceRollResult URGameplayAbilityBase::RollAndBroadcast(AActor* Source, AActor* Target,
	const FDiceExpression& Expression, FGameplayTag MagnitudeTag, EDiceRollContext RollContext)
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI)
	{
		return FDiceRollResult();
	}

	FDiceRollResult Result;
	if (UDiceSubsystem* Dice = GI->GetSubsystem<UDiceSubsystem>())
	{
		Result = Dice->RollExpression(Expression, RollContext);
	}

	FAbilityResolved Event;
	Event.Source       = Source;
	Event.Target       = Target;
	Event.MagnitudeTag = MagnitudeTag;
	Event.Expression   = Expression;
	Event.Result       = Result;
	Event.RollContext  = RollContext;
	Event.AbilityClass = GetClass();

	if (UREventBusSubsystem* EB = GI->GetSubsystem<UREventBusSubsystem>())
	{
		EB->Publish<FAbilityResolved>(Event);
	}

	return Result;
}

FDiceRollResult URGameplayAbilityBase::ApplyEffectWithRoll(AActor* Source, AActor* Target,
	TSubclassOf<UGameplayEffect> EffectClass, const FDiceExpression& Expression, FGameplayTag MagnitudeTag,
	EDiceRollContext RollContext, float Multiplier, int32 DurationTurns)
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI)
	{
		return FDiceRollResult();
	}

	FDiceRollResult Result;
	if (UDiceSubsystem* Dice = GI->GetSubsystem<UDiceSubsystem>())
	{
		Result = Dice->RollExpression(Expression, RollContext);
	}

	// Apply multiplier before broadcasting so all listeners see the final value.
	Result.Total = FMath::Max(0, FMath::RoundToInt(static_cast<float>(Result.Total) * Multiplier));

	FAbilityResolved Event;
	Event.Source       = Source;
	Event.Target       = Target;
	Event.MagnitudeTag = MagnitudeTag;
	Event.Expression   = Expression;
	Event.Result       = Result;
	Event.RollContext  = RollContext;
	Event.AbilityClass = GetClass();

	if (UREventBusSubsystem* EB = GI->GetSubsystem<UREventBusSubsystem>())
	{
		EB->Publish<FAbilityResolved>(Event);
	}

	if (!EffectClass || !Target)
	{
		return Result;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!TargetASC || !SourceASC)
	{
		return Result;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(EffectClass);
	if (!SpecHandle.IsValid())
	{
		return Result;
	}

	if (MagnitudeTag.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(MagnitudeTag, static_cast<float>(Result.Total));
	}
	if (DurationTurns > 0)
	{
		SpecHandle.Data->SetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag("Data.Effect.TurnsLeft"),
			static_cast<float>(DurationTurns));
	}

	const FActiveGameplayEffectHandle AppliedHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	// Instant GEs always return an invalid handle (they execute and vanish — nothing persists).
	const UGameplayEffect* GEDefObj = EffectClass.GetDefaultObject();
	const bool bIsInstant = GEDefObj && GEDefObj->DurationPolicy == EGameplayEffectDurationType::Instant;
	if (!AppliedHandle.IsValid() && !bIsInstant)
	{
		UE_LOG(LogHAL, Warning, TEXT("ApplyEffectWithRoll: %s → FAILED to apply"), *EffectClass->GetName());
	}
	else
	{
		UE_LOG(LogHAL, Log, TEXT("ApplyEffectWithRoll: %s → %s (DurationTurns=%d)"),
			*EffectClass->GetName(),
			bIsInstant ? TEXT("INSTANT executed") : TEXT("APPLIED"),
			DurationTurns);
	}

	return Result;
}

// ────────────── Combat Resolution ──────────────

static bool IsNatural20(const FDiceRollResult& Roll)
{
	return Roll.IndividualRolls.Num() > 0 && Roll.IndividualRolls[0] == 20;
}

int32 URGameplayAbilityBase::StatModifier(float RawStat)
{
	return FMath::FloorToInt((RawStat - 10.f) / 2.f);
}

int32 URGameplayAbilityBase::GetDodgeValue(AActor* Target) const
{
	if (!Target)
	{
		return 10;
	}
	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!ASC)
	{
		return 10;
	}
	const URCharacterCoreAttributeSet* Core = ASC->GetSet<URCharacterCoreAttributeSet>();
	if (!Core)
	{
		return 10;
	}
	int32 Dodge = 10 + StatModifier(Core->GetFinesse());

	// Apply DefenseRollModifier (e.g. Destabilized gives -1 Dodge).
	if (const URCharacterDebuffAttributeSet* Debuffs = ASC->GetSet<URCharacterDebuffAttributeSet>())
	{
		Dodge += FMath::RoundToInt(Debuffs->GetDefenseRollModifier());
	}

	return Dodge;
}

float URGameplayAbilityBase::GetResistanceStat(AActor* Target, EResistanceType Type) const
{
	if (!Target)
	{
		return 10.f;
	}
	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!ASC)
	{
		return 10.f;
	}
	const URCharacterCoreAttributeSet* Core = ASC->GetSet<URCharacterCoreAttributeSet>();
	if (!Core)
	{
		return 10.f;
	}
	switch (Type)
	{
	case EResistanceType::Physical:        return Core->GetPhysique();
	case EResistanceType::Network:         return Core->GetTech();
	case EResistanceType::Mental:          return Core->GetMind();
	case EResistanceType::Radiation:       return Core->GetInsight();
	case EResistanceType::Electromagnetic: return Core->GetFinesse();
	default:                               return 10.f;
	}
}

void URGameplayAbilityBase::BroadcastCheckResolved(
	AActor* Src, AActor* Tgt, FGameplayTag Tag,
	const FDiceExpression& Expr, const FDiceRollResult& Roll,
	int32 TargetValue, bool bSuccess, bool bCrit)
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI)
	{
		return;
	}
	UREventBusSubsystem* EB = GI->GetSubsystem<UREventBusSubsystem>();
	if (!EB)
	{
		return;
	}

	FCheckResolved Event;
	Event.Source       = Src;
	Event.Target       = Tgt;
	Event.CheckTag     = Tag;
	Event.Expression   = Expr;
	Event.Roll         = Roll;
	Event.TargetValue  = TargetValue;
	Event.bSuccess     = bSuccess;
	Event.bCritical    = bCrit;
	Event.AbilityClass = GetClass();

	EB->Publish<FCheckResolved>(Event);
}

FHitCheckResult URGameplayAbilityBase::PerformHitCheck(
	AActor* Source, AActor* Target, int32 AttackBonus, EDiceRollContext Ctx)
{
	FDiceTerm D20;
	D20.Count = 1;
	D20.Die   = EDiceType::D20;

	FDiceExpression Expr;
	Expr.Terms.Add(D20);
	Expr.FlatModifier = AttackBonus;

	FDiceRollResult Roll;
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (GI)
	{
		if (UDiceSubsystem* Dice = GI->GetSubsystem<UDiceSubsystem>())
		{
			Roll = Dice->RollExpression(Expr, Ctx);
		}
	}

	// Apply attacker's AttackRollModifier (e.g. Disrupted Senses gives -2).
	if (Source)
	{
		if (const UAbilitySystemComponent* SrcASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Source))
		{
			if (const URCharacterDebuffAttributeSet* Debuffs = SrcASC->GetSet<URCharacterDebuffAttributeSet>())
			{
				Roll.Total += FMath::RoundToInt(Debuffs->GetAttackRollModifier());
			}
		}
	}

	const int32 Dodge = GetDodgeValue(Target);

	EHitOutcome Outcome;
	if (IsNatural20(Roll))
	{
		Outcome = EHitOutcome::CriticalHit;
	}
	else if (Roll.Total >= Dodge)
	{
		Outcome = EHitOutcome::Hit;
	}
	else
	{
		Outcome = EHitOutcome::Miss;
	}

	const bool bSuccess = (Outcome != EHitOutcome::Miss);
	const bool bCrit    = (Outcome == EHitOutcome::CriticalHit);

	BroadcastCheckResolved(Source, Target,
		FGameplayTag::RequestGameplayTag("Check.Hit"),
		Expr, Roll, Dodge, bSuccess, bCrit);

	UE_LOG(LogHAL, Log, TEXT("[HitCheck] Roll=%d vs Dodge=%d -> %s%s"),
		Roll.Total, Dodge,
		bSuccess ? TEXT("HIT") : TEXT("MISS"),
		bCrit    ? TEXT(" (CRITICAL)") : TEXT(""));

	FHitCheckResult Result;
	Result.Outcome    = Outcome;
	Result.Roll       = Roll;
	Result.TargetDodge = Dodge;
	return Result;
}

FSaveCheckResult URGameplayAbilityBase::PerformSavingThrow(
	AActor* Source, AActor* Target, EResistanceType Type, int32 DC, EDiceRollContext Ctx)
{
	const float RawStat = GetResistanceStat(Target, Type);
	int32 StatMod = StatModifier(RawStat);

	// Apply SaveRollModifier (all saves) and TechSaveModifier (Network/Tech saves only).
	if (Target)
	{
		if (const UAbilitySystemComponent* TgtASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
		{
			if (const URCharacterDebuffAttributeSet* Debuffs = TgtASC->GetSet<URCharacterDebuffAttributeSet>())
			{
				StatMod += FMath::RoundToInt(Debuffs->GetSaveRollModifier());
				if (Type == EResistanceType::Network || Type == EResistanceType::Electromagnetic)
				{
					StatMod += FMath::RoundToInt(Debuffs->GetTechSaveModifier());
				}
			}
		}
	}

	FDiceTerm D20;
	D20.Count = 1;
	D20.Die   = EDiceType::D20;

	FDiceExpression Expr;
	Expr.Terms.Add(D20);
	Expr.FlatModifier = StatMod;

	FDiceRollResult Roll;
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (GI)
	{
		if (UDiceSubsystem* Dice = GI->GetSubsystem<UDiceSubsystem>())
		{
			Roll = Dice->RollExpression(Expr, Ctx);
		}
	}

	ESaveOutcome Outcome;
	if (IsNatural20(Roll))
	{
		Outcome = ESaveOutcome::CriticalSuccess;
	}
	else if (Roll.Total >= DC)
	{
		Outcome = ESaveOutcome::Success;
	}
	else
	{
		Outcome = ESaveOutcome::Fail;
	}

	FGameplayTag CheckTag;
	switch (Type)
	{
	case EResistanceType::Physical:        CheckTag = FGameplayTag::RequestGameplayTag("Check.Save.Physical");        break;
	case EResistanceType::Network:         CheckTag = FGameplayTag::RequestGameplayTag("Check.Save.Network");         break;
	case EResistanceType::Mental:          CheckTag = FGameplayTag::RequestGameplayTag("Check.Save.Mental");          break;
	case EResistanceType::Radiation:       CheckTag = FGameplayTag::RequestGameplayTag("Check.Save.Radiation");       break;
	case EResistanceType::Electromagnetic: CheckTag = FGameplayTag::RequestGameplayTag("Check.Save.Electromagnetic"); break;
	default:                               CheckTag = FGameplayTag::RequestGameplayTag("Check.Save");                 break;
	}

	const bool bSuccess = (Outcome != ESaveOutcome::Fail);
	const bool bCrit    = (Outcome == ESaveOutcome::CriticalSuccess);

	BroadcastCheckResolved(Source, Target, CheckTag, Expr, Roll, DC, bSuccess, bCrit);

	UE_LOG(LogHAL, Log, TEXT("[SavingThrow] Roll=%d vs DC=%d -> %s%s"),
		Roll.Total, DC,
		bSuccess ? TEXT("SUCCESS") : TEXT("FAIL"),
		bCrit    ? TEXT(" (CRITICAL SUCCESS)") : TEXT(""));

	FSaveCheckResult Result;
	Result.Outcome  = Outcome;
	Result.Roll     = Roll;
	Result.AbilityDC = DC;
	return Result;
}

FSaveCheckResult URGameplayAbilityBase::PerformSavingThrowForEffect(
	AActor* Source, AActor* Target,
	TSubclassOf<UGameplayEffect> EffectClass, EDiceRollContext Ctx)
{
	bool bRequiresSave = false;
	EResistanceType ResType = EResistanceType::Physical;
	int32 OutDC = 10;

	if (!URStatusGameplayEffect::GetSaveInfo(EffectClass, bRequiresSave, ResType, OutDC))
	{
		// Not a status effect — return a Fail result so caller still applies the effect.
		FSaveCheckResult Fallback;
		Fallback.Outcome = ESaveOutcome::Fail;
		return Fallback;
	}

	if (!bRequiresSave)
	{
		FSaveCheckResult NoSave;
		NoSave.Outcome = ESaveOutcome::Fail;
		return NoSave;
	}

	const int32 ComputedDC = URStatusGameplayEffect::ComputeDC(EffectClass, Source);
	return PerformSavingThrow(Source, Target, ResType, ComputedDC, Ctx);
}

TArray<AActor*> URGameplayAbilityBase::CollectActorsInRadius(
	const FVector& Center, float Radius, bool bIncludeAllies, bool bIncludeEnemies)
{
	TArray<AActor*> Result;

	AActor* Self = CurrentActorInfo ? CurrentActorInfo->AvatarActor.Get() : nullptr;
	if (!Self)
	{
		return Result;
	}

	const URTurnComponent* SelfTurn = Self->GetComponentByClass<URTurnComponent>();
	const ERCombatTeam SelfTeam = SelfTurn ? SelfTurn->Team : ERCombatTeam::Neutral;

	TArray<AActor*> Overlaps;
	UKismetSystemLibrary::SphereOverlapActors(
		Self,
		Center,
		Radius,
		TArray<TEnumAsByte<EObjectTypeQuery>>{ UEngineTypes::ConvertToObjectType(ECC_Pawn) },
		ARCharacterBase::StaticClass(),
		TArray<AActor*>{ Self },
		Overlaps);

	for (AActor* Actor : Overlaps)
	{
		const URTurnComponent* TurnComp = Actor->GetComponentByClass<URTurnComponent>();
		if (!TurnComp)
		{
			continue;
		}

		const bool bIsAlly  = (TurnComp->Team == SelfTeam);
		const bool bIsEnemy = !bIsAlly;

		if ((bIsAlly && bIncludeAllies) || (bIsEnemy && bIncludeEnemies))
		{
			Result.Add(Actor);
		}
	}

	return Result;
}
