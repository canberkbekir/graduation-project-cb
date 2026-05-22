


#include "AI/EnemyCharacterBase.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Components/RTurnComponent.h"
#include "NavAreas/NavArea_Default.h"

void AEnemyCharacterBase::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AppendTags(EnemyTags);
}

AEnemyCharacterBase::AEnemyCharacterBase()
{
	HAIBaseComponent = CreateDefaultSubobject<UHAIBaseComponent>(TEXT("HAIBaseComponent"));
	CoverComponent = CreateDefaultSubobject<UCoverComponent>(TEXT("CoverComponent"));
	SelectionComponent = CreateDefaultSubobject<URSelectionComponent>(TEXT("SelectionComponent"));
	NavModifierComponent = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifierComponent"));
	NavModifierComponent->AreaClassToReplace = UNavArea_Default::StaticClass();
	NavModifierComponent->FailsafeExtent = FVector(30.0f, 30.0f, 30.0f);
	NavModifierComponent->bIncludeAgentHeight = true;
	NavModifierComponent->SetCanEverAffectNavigation(true);
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AHAIController::StaticClass();
	
	bIsItNPC = true;
}

void AEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HAIBaseComponent)
	{
		HAIBaseComponent->OnPerceptionConfigUpdated.AddDynamic(this, &AEnemyCharacterBase::OnPerceptionConfigUpdated);
		HAIBaseComponent->OnSenseActorForgotten.AddDynamic(this, &AEnemyCharacterBase::OnSenseActorForgotten);
	}
	if (TurnComponent)
	{
		TurnComponent->OnTurnStarted.AddDynamic(this, &AEnemyCharacterBase::OnTurnStarted);
		TurnComponent->OnTurnEnded.AddDynamic(this, &AEnemyCharacterBase::OnTurnEnded);
	}
	if (CoverComponent)
	{
		CoverComponent->OnPositionChanged.AddDynamic(this, &AEnemyCharacterBase::OutFromCover);
		CoverComponent->OnCoverOccupied.AddDynamic(this, &AEnemyCharacterBase::OnCoverOccupied);
		CoverComponent->OnCoverFreed.AddDynamic(this, &AEnemyCharacterBase::OnCoverFreed);
	}
}

void AEnemyCharacterBase::OnTurnStarted()
{
	HAIBaseComponent->GetHAIController()->StartBehaviorTree();
	HAIBaseComponent->GetHAIController()->GetBlackboardComponent()->SetValueAsObject("TargetActor", HAIBaseComponent->GetHAIController()->GetTargetActor());
	HAIBaseComponent->GetHAIController()->GetBlackboardComponent()->SetValueAsClass("ScramblerAbility", ScramblerAbility);
}

void AEnemyCharacterBase::OnTurnEnded()
{
	HAIBaseComponent->GetHAIController()->StopBehaviorTree();
}

void AEnemyCharacterBase::OutFromCover(ECoverType CoverType, uint8 NewCoverPosition)
{
	switch (CoverType)
	{
		case ECoverType::CrouchCover:
			if (NewCoverPosition != static_cast<uint8>(ECrouchCoverPositions::Idle))
			break;
	}
}

void AEnemyCharacterBase::OnCoverOccupied(ACoverPoint* NewCoverPoint)
{
	if (!HAIBaseComponent->GetHAIController()->GetBlackboardComponent())
		return;
	if (HAIBaseComponent->GetHAIController()->GetTargetActor())
		HAIBaseComponent->GetHAIController()->GetBlackboardComponent()->SetValueAsVector("LastSeenTargetPosition", HAIBaseComponent->GetHAIController()->GetTargetActor()->GetActorLocation());
	else if (!HAIBaseComponent->GetHAIController()->TargetLocation.IsZero())
		HAIBaseComponent->GetHAIController()->GetBlackboardComponent()->SetValueAsVector("LastSeenTargetPosition", HAIBaseComponent->GetHAIController()->TargetLocation);
}

void AEnemyCharacterBase::OnCoverFreed(ACoverPoint* FreedCoverPoint)
{
	if (HAIBaseComponent->GetHAIController()->GetBlackboardComponent())
		HAIBaseComponent->GetHAIController()->GetBlackboardComponent()->SetValueAsObject("CoverPoint", nullptr);
}

void AEnemyCharacterBase::OnPerceptionConfigUpdated(const F_SenseData& SenseData)
{
	auto aiController = HAIBaseComponent->GetHAIController();
	if (!aiController)
		return;
	
	auto senseStimulus = SenseData.stimulus;
	if (senseStimulus.WasSuccessfullySensed())
	{
		if (SenseData.sensedActor.IsValid() && Cast<ARCharacterBase>(SenseData.sensedActor.Get()) && SenseData.sensedActor.Get() != this && !Cast<AEnemyCharacterBase>(SenseData.sensedActor.Get()))
		{
			aiController->SetTargetActor(SenseData.sensedActor.Get(), SenseData.senseType, senseStimulus);
			aiController->SetTargetLocation(senseStimulus.StimulusLocation, SenseData.senseType, senseStimulus);
			if (HAIBaseComponent->GetHAIController()->GetBlackboardComponent() && TurnComponent->TurnState == ETurnState::InTurn)
				HAIBaseComponent->GetHAIController()->GetBlackboardComponent()->SetValueAsObject("TargetActor", HAIBaseComponent->GetHAIController()->GetTargetActor());
		}
	}
	else
	{
		if (aiController->GetTargetActor() == SenseData.sensedActor.Get())
		{
			if (HAIBaseComponent->GetHAIController()->GetBlackboardComponent())
				HAIBaseComponent->GetHAIController()->GetBlackboardComponent()->SetValueAsVector("LastSeenTargetPosition", aiController->GetTargetActor() ? aiController->GetTargetActor()->GetActorLocation() : FVector::ZeroVector);
			aiController->SetNullTargetActor();
			//aiController->SetNullTargetLocation();
		}
	}
}

void AEnemyCharacterBase::OnSenseActorForgotten(const AActor* ForgottenActor)
{
	auto aiController = HAIBaseComponent->GetHAIController();
	if (!aiController)
		return;
	
	/*if (aiController->GetTargetActor() == ForgottenActor)
	{
		aiController->SetNullTargetActor();
		aiController->SetNullTargetLocation();
	}*/
	FVector forgottenLocation = ForgottenActor->GetActorLocation();
	FVector currentTargetLocation = FVector::ZeroVector;
	if (aiController->GetTargetActor())
		currentTargetLocation = aiController->GetTargetActor()->GetActorLocation();
	if (!currentTargetLocation.IsZero() && currentTargetLocation.Equals(forgottenLocation))
		aiController->SetNullTargetLocation();
}
