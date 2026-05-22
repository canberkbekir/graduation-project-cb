#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "Core/RGameplayAbilityBase.h"
#include "Events/WorldViewEvents.h"
#include "UI/Models/RActionBarModels.h"
#include "RActionBarWidget.generated.h"

class UUniformGridPanel;
class UImage;
class URActionSlotWidget;
class UAbilitySystemComponent;
class UDataTable;
class ARCharacterBase;
struct FTurnStarted;
struct FAbilityGranted;
struct FAbilityRemoved;
struct FSelectedCharacterChanged;
struct FOnAttributeChangeData;
struct FWorldViewTogglePreBegin;
struct FWorldViewToggleCancelled;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionBarSlotClicked,
	int32, SlotIndex, FGameplayTag, AbilityTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionBarRefreshed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilitySelected,
	FGameplayTag, AbilityTag, bool, bRequiresTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilityDeselected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnCharacterChanged, ARCharacterBase*, NewCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionPointsChanged, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWalkDistanceChanged, float, NewValue, float, MaxValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionBarWorldViewWillChange, EWorldView, FromView, EWorldView, ToView);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnActionBarWorldViewChanged,    EWorldView, NewView);
DECLARE_DYNAMIC_MULTICAST_DELEGATE         (FOnActionBarWorldViewChangeCancelled);

UCLASS(BlueprintType, Blueprintable)
class RIZZGAME_API URActionBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// === DELEGATES ===

	UPROPERTY(BlueprintAssignable, Category="ActionBar|Events")
	FOnActionBarSlotClicked OnSlotClicked;

	UPROPERTY(BlueprintAssignable, Category="ActionBar|Events")
	FOnActionBarRefreshed OnActionBarRefreshed;

	/** Fires when an ability is selected from the bar */
	UPROPERTY(BlueprintAssignable, Category="ActionBar|Events")
	FOnAbilitySelected OnAbilitySelected;

	/** Fires when the current ability selection is cleared */
	UPROPERTY(BlueprintAssignable, Category="ActionBar|Events")
	FOnAbilityDeselected OnAbilityDeselected;

	/** Fires when the action bar switches to a different character (turn change or selection change) */
	UPROPERTY(BlueprintAssignable, Category="ActionBar|Events")
	FOnTurnCharacterChanged OnTurnCharacterChanged;

	/** Fires whenever the active character's Action Points attribute changes */
	UPROPERTY(BlueprintAssignable, Category="ActionBar|Events")
	FOnActionPointsChanged OnActionPointsChanged;

	/** Fires whenever the active character's Walk Distance attribute changes */
	UPROPERTY(BlueprintAssignable, Category="ActionBar|Events")
	FOnWalkDistanceChanged OnWalkDistanceChanged;

	/** Fires before slot swap begins — use for fade-out animations */
	UPROPERTY(BlueprintAssignable, Category="ActionBar|Events|WorldView")
	FOnActionBarWorldViewWillChange OnWorldViewWillChange;

	/** Fires after slot swap completes — use for fade-in animations */
	UPROPERTY(BlueprintAssignable, Category="ActionBar|Events|WorldView")
	FOnActionBarWorldViewChanged OnWorldViewChanged;

	/** Fires if a toggle is cancelled before the swap was applied */
	UPROPERTY(BlueprintAssignable, Category="ActionBar|Events|WorldView")
	FOnActionBarWorldViewChangeCancelled OnWorldViewChangeCancelled;

	// === PUBLIC API ===

	/** Set the display table to use for ability lookups. Call once at setup. */
	UFUNCTION(BlueprintCallable, Category="ActionBar")
	void SetDisplayTable(UDataTable* InDisplayTable);

	/** Bind to a specific ASC and refresh. Called automatically on turn change. */
	UFUNCTION(BlueprintCallable, Category="ActionBar")
	void InitForAbilitySystem(UAbilitySystemComponent* InASC, UDataTable* InDisplayTable);

	/** Full rebuild: re-query ASC abilities and repopulate all slots */
	UFUNCTION(BlueprintCallable, Category="ActionBar")
	void RefreshAllSlots();

	/** Lightweight: re-evaluate slot states without rebuilding widgets */
	UFUNCTION(BlueprintCallable, Category="ActionBar")
	void UpdateSlotStates();

	/** Clear all slot data (keeps empty slot widgets) */
	UFUNCTION(BlueprintCallable, Category="ActionBar")
	void ClearAllSlots();

	/** Swap the abilities in two slot positions */
	UFUNCTION(BlueprintCallable, Category="ActionBar")
	void SwapSlots(int32 IndexA, int32 IndexB);

	/** Move a specific ability to a target slot index */
	UFUNCTION(BlueprintCallable, Category="ActionBar")
	void MoveAbilityToSlot(FGameplayTag AbilityTag, int32 TargetSlotIndex);

	/**
	 * Clear the current ability selection.
	 * @param bIsCancelled  True when the user explicitly cancelled (right-click, new ability selected).
	 *                      False when called internally after executing an ability — lets an in-flight
	 *                      approach walk continue to completion instead of aborting it.
	 */
	UFUNCTION(BlueprintCallable, Category="ActionBar")
	void ClearSelection(bool bIsCancelled = false);

	/** Get the currently selected ability tag (invalid if nothing selected) */
	UFUNCTION(BlueprintPure, Category="ActionBar")
	FGameplayTag GetSelectedAbilityTag() const { return SelectedAbilityTag; }

	/** Is an ability currently selected? */
	UFUNCTION(BlueprintPure, Category="ActionBar")
	bool HasSelection() const { return SelectedAbilityTag.IsValid(); }

	/** Does the currently selected ability require a target? */
	UFUNCTION(BlueprintPure, Category="ActionBar")
	bool SelectedAbilityRequiresTarget() const { return bSelectedRequiresTarget; }

	/** Get the targeting type of the currently selected ability. Returns SelfCast if nothing is selected. */
	UFUNCTION(BlueprintPure, Category="ActionBar")
	ETargetingType GetSelectedTargetingType() const;

	/** Try to execute the selected ability. Pass the resolved hit for targeted abilities (SingleActor/AOE);
	 *  leave default for SelfCast abilities that fire without a world target. */
	UFUNCTION(BlueprintCallable, Category="ActionBar", meta=(AutoCreateRefTerm="TargetHit"))
	bool TryExecuteSelectedAbility(const FHitResult& TargetHit);

	UFUNCTION(BlueprintPure, Category="ActionBar")
	TArray<URActionSlotWidget*> GetSlots() const;

	UFUNCTION(BlueprintPure, Category="ActionBar")
	int32 GetSlotCount() const { return SlotWidgets.Num(); }

	// === ATTRIBUTE GETTERS ===

	/** Current Action Points of the active character. Returns 0 if no character is bound. */
	UFUNCTION(BlueprintPure, Category="ActionBar|Attributes")
	float GetCurrentActionPoints() const;

	/** Current remaining Walk Distance of the active character. Returns 0 if no character is bound. */
	UFUNCTION(BlueprintPure, Category="ActionBar|Attributes")
	float GetCurrentWalkDistance() const;

	/** Max Walk Distance of the active character for this turn. Returns 0 if no character is bound. */
	UFUNCTION(BlueprintPure, Category="ActionBar|Attributes")
	float GetMaxWalkDistance() const;

	/** The character whose abilities are currently shown in the bar. Null if the bar is empty. */
	UFUNCTION(BlueprintPure, Category="ActionBar|Character")
	ARCharacterBase* GetCurrentCharacter() const { return CachedCharacter.Get(); }

protected:
	// BP events
	UFUNCTION(BlueprintImplementableEvent, Category="ActionBar")
	void BP_OnActionBarRefreshed();

	UFUNCTION(BlueprintImplementableEvent, Category="ActionBar")
	void BP_OnSlotClicked(int32 SlotIndex, const FGameplayTag& AbilityTag);

	/** Called when the bar switches to a new character (turn or selection change). */
	UFUNCTION(BlueprintImplementableEvent, Category="ActionBar|Events")
	void BP_OnTurnCharacterChanged(ARCharacterBase* NewCharacter);

	/** Called whenever the active character's Action Points change. */
	UFUNCTION(BlueprintImplementableEvent, Category="ActionBar|Events")
	void BP_OnActionPointsChanged(float NewAP);

	/** Called whenever the active character's Walk Distance changes. */
	UFUNCTION(BlueprintImplementableEvent, Category="ActionBar|Events")
	void BP_OnWalkDistanceChanged(float NewWalkDist, float MaxWalkDist);

	/** Called before slots are swapped; play your fade-out here. */
	UFUNCTION(BlueprintImplementableEvent, Category="ActionBar|Events|WorldView")
	void BP_OnWorldViewWillChange(EWorldView FromView, EWorldView ToView);

	/** Called after slots are swapped; play your fade-in here. */
	UFUNCTION(BlueprintImplementableEvent, Category="ActionBar|Events|WorldView")
	void BP_OnWorldViewChanged(EWorldView NewView);

	/** Called when a toggle is cancelled before the swap was applied. */
	UFUNCTION(BlueprintImplementableEvent, Category="ActionBar|Events|WorldView")
	void BP_OnWorldViewChangeCancelled();

	// === CONFIG ===

	UPROPERTY(meta=(BindWidget))
	UUniformGridPanel* UGP_Slots = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* Img_BarBG = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="ActionBar|Config", meta=(AllowAbstract=false))
	TSubclassOf<URActionSlotWidget> ActionSlotClass;

	UPROPERTY(EditAnywhere, Category="ActionBar|Config", meta=(ClampMin="1", UIMin="1"))
	int32 Columns = 5;

	UPROPERTY(EditAnywhere, Category="ActionBar|Config")
	TObjectPtr<UDataTable> DisplayTable;

	UPROPERTY(EditAnywhere, Category="Design Time")
	bool bDesignPreview = true;

	// Lifecycle
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	/** Creates exactly Columns slot widgets in the grid (once). */
	void CreateEmptySlots();

	/** Fills existing slots with ability data, leaves the rest empty. */
	void FillSlotsFromViewModels(const TArray<FActionSlotViewModel>& ViewModels);

	void UnbindAllSlotEvents();
	void BindAllSlotEvents();

	UFUNCTION()
	void HandleSlotInteracted(int32 SlotIndex, FGameplayTag AbilityTag);

	// EventBus handlers
	void OnTurnStarted(const FTurnStarted& Event);
	void OnAbilityGranted(const FAbilityGranted& Event);
	void OnAbilityRemoved(const FAbilityRemoved& Event);
	void OnSelectedCharacterChanged(const FSelectedCharacterChanged& Event);

	/** Resolves the selected party character's ASC and refreshes. */
	void RefreshForSelectedCharacter();

	void BindAttributeCallbacks(UAbilitySystemComponent* ASC);
	void UnbindAttributeCallbacks();
	void OnActionPointsAttributeChanged(const FOnAttributeChangeData& Data);
	void OnWalkDistanceAttributeChanged(const FOnAttributeChangeData& Data);

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	UPROPERTY()
	TWeakObjectPtr<ARCharacterBase> CachedCharacter;

	UPROPERTY()
	TArray<TObjectPtr<URActionSlotWidget>> SlotWidgets;

	bool bSlotsCreated = false;

	UPROPERTY()
	FGameplayTag SelectedAbilityTag;

	bool bSelectedRequiresTarget = false;
	int32 SelectedSlotIndex = -1;

	FHitResult PendingTargetHit;
	FDelegateHandle APChangedHandle;
	FDelegateHandle WalkDistChangedHandle;

	EWorldView CurrentWorldView = EWorldView::Physical;

	void OnWorldViewTogglePreBegin(const FWorldViewTogglePreBegin& Event);
	void HandleWorldViewChanged(const FWorldViewChanged& Event);
	void OnWorldViewToggleCancelled(const FWorldViewToggleCancelled& Event);

public:
	const FHitResult& GetPendingTargetHit() const { return PendingTargetHit; }

	/** ASC'den ability tag'ine göre instance bulur */
	URGameplayAbilityBase* FindAbilityInstance(FGameplayTag AbilityTag) const;

	/** Şu an seçili ability instance'ı (deselect sırasında çağırmak için cache) */
	UPROPERTY()
	TWeakObjectPtr<URGameplayAbilityBase> SelectedAbilityInstance;
};
