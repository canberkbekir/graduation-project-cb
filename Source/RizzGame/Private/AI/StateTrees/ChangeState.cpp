#include "AI/StateTrees/ChangeState.h"

#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"
#include "Components/RTurnComponent.h"
#include "Core/RCharacterBase.h"
#include "Core/RConversions.h"
#include "Character/RCharacterSpec.h"
#include "GAS/Attributes/RCharacterTurnAttributeSet.h"


EStateTreeRunStatus FChangeState::EnterState(FStateTreeExecutionContext& Context,
                                             const FStateTreeTransitionResult& Transition) const
{
	const FChangeStateData* Data = Context.GetInstanceDataPtr<FChangeStateData>(*this);

	UE_LOG(LogTemp, Log, TEXT("Custom Task %s"), *Data->Actor->GetName());


	URTurnComponent* TurnComponent = Data->Actor->FindComponentByClass<URTurnComponent>();
	UAbilitySystemComponent* Asc = Cast<ARCharacterBase>(Data->Actor)->GetAbilitySystemComponent();
	if (TurnComponent)
	{
		TurnComponent->TurnState = Data->TurnState;
		switch (TurnComponent->TurnState)
		{
		case ETurnState::InTurn:
			Asc->ApplyGameplayEffectToSelf(Data->Effect.GetDefaultObject(), 1.0f, Asc->MakeEffectContext());
			if (const ARCharacterBase* Char = Cast<ARCharacterBase>(Data->Actor))
			{
				if (const FCharacterDefinitionRow* Def = Char->GetCharacterRow().GetRow<FCharacterDefinitionRow>(TEXT("ChangeState")))
				{
					const float WalkUU = RConversions::FeetToUU(Def->MaxWalkDistance);
					Asc->SetNumericAttributeBase(URCharacterTurnAttributeSet::GetActionPointsAttribute(), Def->ActionPoints);
					Asc->SetNumericAttributeBase(URCharacterTurnAttributeSet::GetMaxWalkDistanceAttribute(), WalkUU);
					Asc->SetNumericAttributeBase(URCharacterTurnAttributeSet::GetWalkDistanceAttribute(), WalkUU);
				}
			}
			break;
		case ETurnState::OutTurn:
		//Asc->ApplyGameplayEffectToSelf(Data->Effect.GetDefaultObject(), 1.0f, Asc->MakeEffectContext());
		case ETurnState::Waiting:
			break;
		}
	}
	return EStateTreeRunStatus::Running;
}
