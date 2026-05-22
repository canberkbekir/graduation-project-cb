// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/RCombatScenario.h"
#include "Subsystems/RCombatScenarioSubsystem.h"
#include "Core/RCharacterBase.h"
#include "Components/SceneComponent.h"
#include "Components/BillboardComponent.h"
#include "DrawDebugHelpers.h"

ARCombatScenario::ARCombatScenario()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		SpriteComponent->SetupAttachment(Root);
		SpriteComponent->bIsScreenSizeScaled = true;
	}
#endif

#if WITH_EDITOR
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
#endif
}

void ARCombatScenario::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(false);

	if (URCombatScenarioSubsystem* Subsystem = GetWorld()->GetSubsystem<URCombatScenarioSubsystem>())
	{
		Subsystem->RegisterScenario(this);
	}
}

void ARCombatScenario::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (URCombatScenarioSubsystem* Subsystem = GetWorld()->GetSubsystem<URCombatScenarioSubsystem>())
	{
		Subsystem->UnregisterScenario(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ARCombatScenario::MarkCompleted()
{
	ScenarioState = ERCombatScenarioState::Completed;
}

#if WITH_EDITOR
void ARCombatScenario::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!GetWorld()->IsPlayInEditor())
	{
		const FVector Origin = GetActorLocation();
		for (const TObjectPtr<ARCharacterBase>& Enemy : Enemies)
		{
			if (Enemy)
			{
				DrawDebugLine(GetWorld(), Origin, Enemy->GetActorLocation(), FColor::Red, false, -1.f, SDPG_Foreground, 2.f);
			}
		}
	}
}
#endif // WITH_EDITOR

void ARCombatScenario::ExecutePreCombat()
{
	ScenarioState = ERCombatScenarioState::PreCombat;
	OnPreCombatDelegate.Broadcast();
	OnPreCombat();

	if (!OnPreCombatDelegate.IsBound() &&
		!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(ARCombatScenario, OnPreCombat)))
	{
		FinishPreCombat();
	}
}

void ARCombatScenario::ExecuteStartCombat()
{
	ScenarioState = ERCombatScenarioState::InCombat;
	OnCombatStartDelegate.Broadcast();
	OnCombatStart();
}

void ARCombatScenario::ExecutePostCombat()
{
	ScenarioState = ERCombatScenarioState::PostCombat;
	OnPostCombatDelegate.Broadcast();
	OnPostCombat();

	if (!OnPostCombatDelegate.IsBound() &&
		!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(ARCombatScenario, OnPostCombat)))
	{
		FinishPostCombat();
	}
}

void ARCombatScenario::FinishPreCombat()
{
	if (ScenarioState != ERCombatScenarioState::PreCombat)
	{
		return;
	}

	if (URCombatScenarioSubsystem* Subsystem = GetWorld()->GetSubsystem<URCombatScenarioSubsystem>())
	{
		Subsystem->OnPreCombatFinished(this);
	}
}

void ARCombatScenario::FinishPostCombat()
{
	if (ScenarioState != ERCombatScenarioState::PostCombat)
	{
		return;
	}

	if (URCombatScenarioSubsystem* Subsystem = GetWorld()->GetSubsystem<URCombatScenarioSubsystem>())
	{
		Subsystem->OnPostCombatFinished(this);
	}
}
