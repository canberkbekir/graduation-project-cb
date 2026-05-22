// RCombatInitiativeQueueWidget.cpp

#include "UI/Combat/RCombatInitiativeQueueWidget.h"

#include "Subsystems/REventBusSubsystem.h"
#include "Events/CombatEvents.h"

void URCombatInitiativeQueueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UREventBusSubsystem* Bus = GetGameInstance()->GetSubsystem<UREventBusSubsystem>())
	{
		Bus->Subscribe<FCombatInitiativeChanged>(
			this,
			&URCombatInitiativeQueueWidget::OnInitiativeChanged,
			EReplayMode::Last
		);
	}
}

void URCombatInitiativeQueueWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	HB_Root->ClearChildren();

	for (int32 i = 0; i < MaxVisibleEntries; ++i)
	{
		if (PortraitWidgetClass)
		{
			if (URPortraitWidget* NewPortrait = CreateWidget<URPortraitWidget>(this, PortraitWidgetClass); NewPortrait && HB_Root)
			{
				FPortraitViewModel ViewModel;
				NewPortrait->Init(ViewModel);
				HB_Root->AddChild(NewPortrait);
			}
		}
	}
}

void URCombatInitiativeQueueWidget::NativeDestruct()
{
	Super::NativeDestruct();
	if (UREventBusSubsystem* Bus = GetGameInstance()->GetSubsystem<UREventBusSubsystem>())
		Bus->UnsubscribeAll(this);
}

void URCombatInitiativeQueueWidget::OnInitiativeChanged(const FCombatInitiativeChanged& Event)
{
	if (!HB_Root)
	{
		return;
	}

	HB_Root->ClearChildren();

	const int32 NumCombatants = Event.Combatants.Num();
	if (NumCombatants == 0)
	{
		return;
	}

	int32 PortraitsAdded = 0;
	int32 CurrentRound = Event.RoundNumber;
	int32 WalkIndex = Event.CurrentTurnIndex;

	while (PortraitsAdded < MaxVisibleEntries)
	{
		// Build portrait from combatant info
		const FInitiativeCombatantInfo& Info = Event.Combatants[WalkIndex];

		if (PortraitWidgetClass)
		{
			if (URPortraitWidget* Portrait = CreateWidget<URPortraitWidget>(this, PortraitWidgetClass))
			{
				FPortraitViewModel ViewModel;
				ViewModel.PortraitTexture = Info.Portrait;
				ViewModel.CurrentHealth = Info.CurrentHealth;
				ViewModel.MaxHealth = Info.MaxHealth;
				ViewModel.CurrentKineticShields = Info.CurrentKineticShields;
				ViewModel.MaxKineticShields = Info.MaxKineticShields;
				ViewModel.CurrentEnergyShields = Info.CurrentEnergyShields;
				ViewModel.MaxEnergyShields = Info.MaxEnergyShields;
				ViewModel.Team = Info.Team;
				ViewModel.StatusEffects = Info.StatusEffects;

				Portrait->Init(ViewModel);
				HB_Root->AddChild(Portrait);
				++PortraitsAdded;
			}
		}

		// Advance walk index
		const int32 NextIndex = (WalkIndex + 1) % NumCombatants;

		// If we wrapped around, insert a round separator before the next portrait
		if (NextIndex <= WalkIndex && PortraitsAdded < MaxVisibleEntries)
		{
			++CurrentRound;

			if (bShowRoundText && RoundSeparatorClass)
			{
				if (URRoundSeparatorWidget* Separator = CreateWidget<URRoundSeparatorWidget>(this, RoundSeparatorClass))
				{
					Separator->SetRound(CurrentRound);
					HB_Root->AddChild(Separator);
				}
			}
		}

		WalkIndex = NextIndex;
	}

	UE_LOG(LogTemp, Log, TEXT("Initiative queue: %d combatants, round %d, showing %d portraits"),
		NumCombatants, Event.RoundNumber, PortraitsAdded);
}
