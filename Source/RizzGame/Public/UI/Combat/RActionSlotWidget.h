#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Models/RActionBarModels.h"
#include "GameplayTagContainer.h"
#include "RActionSlotWidget.generated.h"

class UAbilitySystemComponent;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionSlotInteracted,
	int32, SlotIndex, FGameplayTag, AbilityTag);

UCLASS(BlueprintType, Blueprintable)
class RIZZGAME_API URActionSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** ActionBar binds to this. Fires with SlotIndex and AbilityTag on click. */
	UPROPERTY(BlueprintAssignable, Category="ActionSlot|Events")
	FOnActionSlotInteracted OnSlotInteracted;

	/** Populate the slot from a view-model and cache the ASC reference */
	UFUNCTION(BlueprintCallable, Category="ActionSlot")
	void InitSlot(const FActionSlotViewModel& InViewModel, UAbilitySystemComponent* InASC);

	/** Clear the slot back to empty state */
	UFUNCTION(BlueprintCallable, Category="ActionSlot")
	void ClearSlot();

	/** Update state without replacing the whole view-model */
	UFUNCTION(BlueprintCallable, Category="ActionSlot")
	void UpdateSlotState(EActionSlotState NewState, int32 CooldownTurns = 0);

	/** Try to activate this slot's ability via the cached ASC */
	UFUNCTION(BlueprintCallable, Category="ActionSlot")
	bool TryActivateAbility();

	// --- Getters ---

	UFUNCTION(BlueprintPure, Category="ActionSlot")
	FGameplayTag GetAbilityTag() const { return ViewModel.AbilityTag; }

	UFUNCTION(BlueprintPure, Category="ActionSlot")
	const FActionSlotViewModel& GetViewModel() const { return ViewModel; }

	UFUNCTION(BlueprintPure, Category="ActionSlot")
	int32 GetSlotIndex() const { return ViewModel.SlotIndex; }

	UFUNCTION(BlueprintPure, Category="ActionSlot")
	bool IsEmpty() const { return ViewModel.SlotState == EActionSlotState::Empty; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** BP hook: called after InitSlot finishes — set up icon, cooldown overlay, AP text, etc. */
	UFUNCTION(BlueprintImplementableEvent, Category="ActionSlot")
	void BP_OnSlotInitialized(const FActionSlotViewModel& VM);

	/** BP hook: called when slot state changes */
	UFUNCTION(BlueprintImplementableEvent, Category="ActionSlot")
	void BP_OnSlotStateChanged(EActionSlotState NewState, int32 CooldownTurns);

	/** BP hook: called when slot is cleared */
	UFUNCTION(BlueprintImplementableEvent, Category="ActionSlot")
	void BP_OnSlotCleared();

	/** The button that handles click input. Place a UButton named BTN_Slot in your BP. */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget), Category="ActionSlot")
	UButton* BTN_Slot = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="ActionSlot")
	FActionSlotViewModel ViewModel;

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

private:
	UFUNCTION()
	void HandleButtonClicked();
};
