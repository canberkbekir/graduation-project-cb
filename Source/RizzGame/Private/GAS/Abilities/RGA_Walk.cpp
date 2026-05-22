// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/RGA_Walk.h"

#include "AIController.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GAS/Attributes/RCharacterTurnAttributeSet.h"
#include "Navigation/PathFollowingComponent.h"


URGA_Walk::URGA_Walk()
{
	FGameplayTag tag = FGameplayTag::RequestGameplayTag(FName("Ability.Walk"));
	// Only allow activation if it's this character's turn
	//ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag("State.InTurn"));
	//AbilityTags.AddTag(tag);
	SetAssetTags(FGameplayTagContainer(tag));
}

void URGA_Walk::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo,
                                const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogHAL, Log, TEXT("[Walk] ActivateAbility called"));

	if (!ActivationSetup(Handle, ActorInfo, ActivationInfo, TriggerEventData))
	{
		UE_LOG(LogHAL, Warning, TEXT("[Walk] ActivationSetup failed"));
		return;
	}

	// ActivationSetup no longer commits AP — Walk commits it here, same as the original flow.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogHAL, Warning, TEXT("[Walk] CommitAbility failed — not enough AP or cooldown active"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogHAL, Log, TEXT("[Walk] CommitAbility OK"));

	ARCharacterBase* Character = Cast<ARCharacterBase>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		UE_LOG(LogHAL, Warning, TEXT("[Walk] AvatarActor is not ARCharacterBase"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogHAL, Log, TEXT("[Walk] IsCharacterInTurn=%d"), IsCharacterInTurn(Character) ? 1 : 0);

	if (IsCharacterInTurn(Character))
	{
		if (!TurnWalk(Character))
		{
			UE_LOG(LogHAL, Warning, TEXT("[Walk] TurnWalk returned false"));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
	}
	else
	{
		if (!Walk(Character))
		{
			UE_LOG(LogHAL, Warning, TEXT("[Walk] Walk returned false"));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
	}
	if (Character->bIsItNPC)
		return;
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool URGA_Walk::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                   FGameplayTagContainer* OptionalRelevantTags) const
{
	return true;
}


bool URGA_Walk::Walk(const ACharacter* Character)
{
	//UE_LOG(LogTemp, Log, TEXT("normal"));
	if (!Character)
	{
		return false;
	}

	StartLocation = Character->GetActorLocation();

	if (CurrentEventData.Target)
	{
		DesiredTargetLocation = CurrentEventData.Target->GetActorLocation();
	}
	else
	{
		// Set DesiredTargetLocation (e.g., via event or cursor)
		if (!GetHitResultUnderCursor(HitResult))
		{
			return false;
		}
		DesiredTargetLocation = HitResult.Location;
	}

	// Find navigation path to DesiredTargetLocation
	UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(
		this,
		StartLocation,
		DesiredTargetLocation
	);

	if (!Path || !Path->IsValid() || Path->PathPoints.Num() < 2)
	{
		return false;
	}

	const FVector FinalTargetLocation = DesiredTargetLocation;

	// Debug path
	for (int i = 1; i < Path->PathPoints.Num(); i++)
	{
		DrawDebugLine(GetWorld(), Path->PathPoints[i - 1], Path->PathPoints[i], FColor::Blue, false, -1.0f, 0, 5.f);
	}
	DrawDebugSphere(GetWorld(), FinalTargetLocation, 25.f, 12, FColor::Green, false, -1.f);

	// Move to clamped target location
	AAIController* AIController = Cast<AAIController>(Character->GetController());
	if (!AIController)
	{
		return false;
	}

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(FinalTargetLocation);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetAllowPartialPath(true);

	FNavPathSharedPtr NavPath;
	AIController->MoveTo(MoveRequest, &NavPath);

	return true;
}

bool URGA_Walk::TurnWalk(ARCharacterBase* Character)
{
	//UE_LOG(LogTemp, Log, TEXT("turn"));
	if (!Character)
	{
		return false;
	}

	UAbilitySystemComponent* Asc = Character->GetAbilitySystemComponent();
	if (!Asc)
	{
		return false;
	}

	const URCharacterTurnAttributeSet* Attributes = Asc->GetSet<URCharacterTurnAttributeSet>();
	if (!Attributes)
	{
		return false;
	}

	float WalkDistanceLeft = Attributes->GetWalkDistance();
	if (WalkDistanceLeft <= 0.f)
	{
		return false;
	}

	StartLocation = Character->GetActorLocation();
	
	
	if (CurrentEventData.Target)
	{
		DesiredTargetLocation = CurrentEventData.Target->GetActorLocation();
	}
	else if (CurrentEventData.TargetData.Num() > 0 && CurrentEventData.TargetData.Get(0)->HasEndPoint())
	{
		DesiredTargetLocation = CurrentEventData.TargetData.Get(0)->GetEndPointTransform().GetLocation();
	}
	else
	{
		// Set DesiredTargetLocation (you must implement this method or provide it)
		if (!GetHitResultUnderCursor(HitResult))
		{
			return false;
		}
		DesiredTargetLocation = HitResult.Location;
	}

	UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(
		this,
		StartLocation,
		DesiredTargetLocation
	);

	if (!Path || !Path->IsValid() || Path->PathPoints.Num() < 2)
	{
		return false;
	}

	float DistanceAlongPath = 0.f;
	FVector FinalTargetLocation = StartLocation;

	for (int i = 1; i < Path->PathPoints.Num(); i++)
	{
		const FVector Prev = Path->PathPoints[i - 1];
		const FVector Curr = Path->PathPoints[i];
		const float SegmentLength = FVector::Dist(Prev, Curr);

		if (DistanceAlongPath + SegmentLength >= WalkDistanceLeft)
		{
			float Remaining = WalkDistanceLeft - DistanceAlongPath;
			FVector Dir = (Curr - Prev).GetSafeNormal();
			FinalTargetLocation = Prev + Dir * Remaining;
			DistanceAlongPath += Remaining;
			break;
		}
		DistanceAlongPath += SegmentLength;
		FinalTargetLocation = Curr;
	}

	// Update WalkDistanceLeft attribute using ASC (do not modify attribute directly)
	float NewWalkDistanceLeft = FMath::Max(0.f, WalkDistanceLeft - DistanceAlongPath);
	Asc->SetNumericAttributeBase(URCharacterTurnAttributeSet::GetWalkDistanceAttribute(), NewWalkDistanceLeft);

	UE_LOG(LogHAL, Log, TEXT("[Walk] TurnWalk — walked: %.1f | remaining: %.1f"), DistanceAlongPath, NewWalkDistanceLeft);

	// Debug draw
	for (int i = 1; i < Path->PathPoints.Num(); i++)
	{
		DrawDebugLine(GetWorld(), Path->PathPoints[i - 1], Path->PathPoints[i], FColor::Blue, false, 0.0f, 0, 5.f);
	}
	DrawDebugSphere(GetWorld(), FinalTargetLocation, 25.f, 12, FColor::Green, false, 0.f);

	// Move character using AIController
	AAIController* AIController = Cast<AAIController>(Character->GetController());
	if (!AIController)
	{
		return false;
	}


	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(FinalTargetLocation);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetAllowPartialPath(true);

	float FinalRadius = 5.0f;
	if (CurrentEventData.EventMagnitude > 0.0f)
	{
		FinalRadius = CurrentEventData.EventMagnitude;
	}
	MoveRequest.SetAcceptanceRadius(FinalRadius);
	MoveRequest.SetReachTestIncludesAgentRadius(true);

	FNavPathSharedPtr NavPath;
	const FPathFollowingRequestResult MoveResult = AIController->MoveTo(MoveRequest, &NavPath);

	if (MoveResult.Code == EPathFollowingRequestResult::Failed)
	{
		return false;
	}

	if (MoveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		// OnRequestFinished already fired synchronously during MoveTo — no async callback will come.
		// End the ability directly so the BT task doesn't hang.
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return true;
	}

	// Pending: OnRequestFinished fires asynchronously in a future frame, safe to register now.
	if (Character->bIsItNPC)
	{
		if (UPathFollowingComponent* PathComp = AIController->GetPathFollowingComponent())
		{
			CachedAIController = AIController;
			PathComp->OnRequestFinished.Remove(MoveDelegateHandle);
			CachedMoveID = MoveResult.MoveId;
			MoveDelegateHandle = PathComp->OnRequestFinished.AddUObject(this, &URGA_Walk::OnMoveFinished);
		}
	}

	return true;
}

void URGA_Walk::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (CachedAIController)
	{
		if (UPathFollowingComponent* PathComp = CachedAIController->GetPathFollowingComponent())
			PathComp->OnRequestFinished.Remove(MoveDelegateHandle);
		if (bWasCancelled)
			CachedAIController->StopMovement();
		CachedAIController = nullptr;
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URGA_Walk::OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	// Ignore callbacks from other move requests (e.g. an aborted approach walk from a previous ability).
	if (RequestID != CachedMoveID)
	{
		return;
	}

	if (CachedAIController && CachedAIController->GetPathFollowingComponent())
		CachedAIController->GetPathFollowingComponent()->OnRequestFinished.Remove(MoveDelegateHandle);

	bool bWasCancelled = (Result.Code != EPathFollowingResult::Success);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

float URGA_Walk::GetEffectiveRange() const
{
	return MAX_FLT;
}
