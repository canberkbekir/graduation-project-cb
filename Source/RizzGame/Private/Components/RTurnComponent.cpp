// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/RTurnComponent.h"
#include "Core/RPlayerControllerBase.h"
#include "Subsystems/RCombatManagerSubsystem.h"


// Sets default values for this component's properties
URTurnComponent::URTurnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTree"));


	// ...
}


// Called when the game starts
void URTurnComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	StateTreeComponent = GetOwner()->FindComponentByClass<UStateTreeComponent>();
}


// Called every frame
void URTurnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void URTurnComponent::StartTurn()
{
	StateTreeComponent->SendStateTreeEvent(
		FStateTreeEvent(FGameplayTag::RequestGameplayTag("State.Turn.Active"))
	);
	UE_LOG(LogHAL, Log, TEXT("Turn name : %s "), *GetOwner()->GetName());

	OnTurnStarted.Broadcast();
}

void URTurnComponent::EndTurn()
{
	StateTreeComponent->SendStateTreeEvent(
		FStateTreeEvent(FGameplayTag::RequestGameplayTag("State.Turn.Waiting"))
	);
	if (URCombatManagerSubsystem* TurnManagerSubsystem = GetOwner()->GetGameInstance()->GetSubsystem<
		URCombatManagerSubsystem>())
	{
		TurnManagerSubsystem->AdvanceTurn();
	}

	OnTurnEnded.Broadcast();
}

void URTurnComponent::EnterCombat()
{
	StateTreeComponent->SendStateTreeEvent(
		FStateTreeEvent(FGameplayTag::RequestGameplayTag("State.Turn.Waiting"))
	);
	UE_LOG(LogHAL, Log, TEXT(" %s Has enter combat"), *GetOwner()->GetName());
}

void URTurnComponent::ExitCombat()
{
	StateTreeComponent->SendStateTreeEvent(
		FStateTreeEvent(FGameplayTag::RequestGameplayTag("State.Turn.Exited"))
	);
}
