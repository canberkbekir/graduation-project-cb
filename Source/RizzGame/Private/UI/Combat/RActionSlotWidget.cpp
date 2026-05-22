#include "UI/Combat/RActionSlotWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/Button.h"

void URActionSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BTN_Slot)
	{
		BTN_Slot->OnClicked.AddDynamic(this, &URActionSlotWidget::HandleButtonClicked);
	}
}

void URActionSlotWidget::NativeDestruct()
{
	if (BTN_Slot)
	{
		BTN_Slot->OnClicked.RemoveDynamic(this, &URActionSlotWidget::HandleButtonClicked);
	}

	Super::NativeDestruct();
}

void URActionSlotWidget::InitSlot(const FActionSlotViewModel& InViewModel, UAbilitySystemComponent* InASC)
{
	ViewModel = InViewModel;
	CachedASC = InASC;

	if (BTN_Slot)
	{
		BTN_Slot->SetIsEnabled(ViewModel.SlotState == EActionSlotState::Available);
	}

	BP_OnSlotInitialized(ViewModel);
}

void URActionSlotWidget::ClearSlot()
{
	ViewModel = FActionSlotViewModel();
	CachedASC = nullptr;

	if (BTN_Slot)
	{
		BTN_Slot->SetIsEnabled(false);
	}

	BP_OnSlotCleared();
}

void URActionSlotWidget::UpdateSlotState(EActionSlotState NewState, int32 CooldownTurns)
{
	ViewModel.SlotState = NewState;
	ViewModel.CooldownTurnsRemaining = CooldownTurns;

	if (BTN_Slot)
	{
		BTN_Slot->SetIsEnabled(NewState == EActionSlotState::Available);
	}

	BP_OnSlotStateChanged(NewState, CooldownTurns);
}

bool URActionSlotWidget::TryActivateAbility()
{
	if (!CachedASC.IsValid())
	{
		return false;
	}

	if (!ViewModel.AbilityTag.IsValid())
	{
		return false;
	}

	FGameplayTagContainer Tags;
	Tags.AddTag(ViewModel.AbilityTag);
	return CachedASC->TryActivateAbilitiesByTag(Tags);
}

void URActionSlotWidget::HandleButtonClicked()
{
	OnSlotInteracted.Broadcast(ViewModel.SlotIndex, ViewModel.AbilityTag);
}
