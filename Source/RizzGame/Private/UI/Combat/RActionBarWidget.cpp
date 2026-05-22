#include "UI/Combat/RActionBarWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "UI/Combat/RActionSlotWidget.h"
#include "GAS/RAbilityDisplayLibrary.h"
#include "GAS/Attributes/RCharacterTurnAttributeSet.h"
#include "Events/CombatEvents.h"
#include "Events/PartyEvents.h"
#include "Events/WorldViewEvents.h"
#include "Subsystems/REventBusSubsystem.h"
#include "Subsystems/RWorldStateSubsystem.h"
#include "Subsystems/RPartySubsystem.h"
#include "Core/RCharacterBase.h"
#include "Core/RGameplayAbilityBase.h"
#include "Core/RPlayerController.h"
#include "Engine/EngineTypes.h"

// ────────────── Lifecycle ──────────────

void URActionBarWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!UGP_Slots || !ActionSlotClass || Columns <= 0)
	{
		if (UGP_Slots)
		{
			UGP_Slots->ClearChildren();
		}
		return;
	}

	// Design-time preview: fill grid with empty slots
	if (IsDesignTime() && bDesignPreview)
	{
		UGP_Slots->ClearChildren();

		for (int32 i = 0; i < Columns; ++i)
		{
			UUserWidget* Raw = WidgetTree->ConstructWidget<UUserWidget>(ActionSlotClass);
			if (URActionSlotWidget* SlotW = Cast<URActionSlotWidget>(Raw))
			{
				if (auto* GridSlot = UGP_Slots->AddChildToUniformGrid(SlotW, 0, i))
				{
					GridSlot->SetHorizontalAlignment(HAlign_Fill);
					GridSlot->SetVerticalAlignment(VAlign_Fill);
				}
			}
		}
	}
}

void URActionBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Create empty slots on first construct
	CreateEmptySlots();

	// Seed CurrentWorldView from the authoritative subsystem
	if (UWorld* World = GetWorld())
	{
		if (URWorldStateSubsystem* Sub = World->GetSubsystem<URWorldStateSubsystem>())
		{
			CurrentWorldView = Sub->GetCurrentView();
		}
	}

	if (ARPlayerController* PC = Cast<ARPlayerController>(GetOwningPlayer()))
	{
		PC->SetActionBar(this);
	}

	// Subscribe to EventBus
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* Bus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			Bus->Subscribe<FTurnStarted>(this, &URActionBarWidget::OnTurnStarted);
			Bus->Subscribe<FAbilityGranted>(this, &URActionBarWidget::OnAbilityGranted);
			Bus->Subscribe<FAbilityRemoved>(this, &URActionBarWidget::OnAbilityRemoved);
			Bus->Subscribe<FSelectedCharacterChanged>(this, &URActionBarWidget::OnSelectedCharacterChanged);
			Bus->Subscribe<FWorldViewTogglePreBegin> (this, &URActionBarWidget::OnWorldViewTogglePreBegin);
			Bus->Subscribe<FWorldViewChanged>        (this, &URActionBarWidget::HandleWorldViewChanged);
			Bus->Subscribe<FWorldViewToggleCancelled>(this, &URActionBarWidget::OnWorldViewToggleCancelled);
		}
	}

	RefreshForSelectedCharacter();
}

void URActionBarWidget::NativeDestruct()
{
	UnbindAllSlotEvents();
	UnbindAttributeCallbacks();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* Bus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			Bus->UnsubscribeAll(this);
		}
	}

	Super::NativeDestruct();
}

void URActionBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!SelectedAbilityInstance.IsValid())
	{
		return;
	}

	URGameplayAbilityBase* Ability = SelectedAbilityInstance.Get();

	AActor* Avatar = Ability->GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	const FVector SelfLocation = Avatar->GetActorLocation();

	FVector CursorLocation = SelfLocation;
	if (const APlayerController* PC = GetOwningPlayer())
	{
		FHitResult Hit;
		if (PC->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Hit))
		{
			CursorLocation = Hit.Location;
		}
	}

	Ability->TickIndicator(CursorLocation);
}

// ────────────── Public API ──────────────

void URActionBarWidget::SetDisplayTable(UDataTable* InDisplayTable)
{
	DisplayTable = InDisplayTable;
	RefreshForSelectedCharacter();
}

void URActionBarWidget::InitForAbilitySystem(UAbilitySystemComponent* InASC, UDataTable* InDisplayTable)
{
	CachedASC = InASC;
	DisplayTable = InDisplayTable;
	RefreshAllSlots();
}

void URActionBarWidget::RefreshAllSlots()
{
	ClearSelection();

	if (!CachedASC.IsValid() || !DisplayTable)
	{
		// No ASC — clear all slots to empty
		ClearAllSlots();
		return;
	}

	TArray<FActionSlotViewModel> ViewModels =
		URAbilityDisplayLibrary::BuildActionBarViewModels(CachedASC.Get(), DisplayTable, CurrentWorldView);

	FillSlotsFromViewModels(ViewModels);

	OnActionBarRefreshed.Broadcast();
	BP_OnActionBarRefreshed();
}

void URActionBarWidget::UpdateSlotStates()
{
	if (!CachedASC.IsValid() || !DisplayTable)
	{
		return;
	}

	TArray<FActionSlotViewModel> ViewModels =
		URAbilityDisplayLibrary::BuildActionBarViewModels(CachedASC.Get(), DisplayTable, CurrentWorldView);

	for (URActionSlotWidget* SlotWidget : SlotWidgets)
	{
		if (!SlotWidget || SlotWidget->IsEmpty())
		{
			continue;
		}

		const FGameplayTag& SlotTag = SlotWidget->GetAbilityTag();
		for (const FActionSlotViewModel& VM : ViewModels)
		{
			if (VM.AbilityTag.MatchesTagExact(SlotTag))
			{
				SlotWidget->UpdateSlotState(VM.SlotState, VM.CooldownTurnsRemaining);
				break;
			}
		}
	}
}

void URActionBarWidget::ClearAllSlots()
{
	for (URActionSlotWidget* SlotWidget : SlotWidgets)
	{
		if (SlotWidget)
		{
			SlotWidget->ClearSlot();
		}
	}
}

void URActionBarWidget::SwapSlots(int32 IndexA, int32 IndexB)
{
	if (!SlotWidgets.IsValidIndex(IndexA) || !SlotWidgets.IsValidIndex(IndexB))
	{
		return;
	}

	FActionSlotViewModel TempA = SlotWidgets[IndexA]->GetViewModel();
	FActionSlotViewModel TempB = SlotWidgets[IndexB]->GetViewModel();

	TempA.SlotIndex = IndexB;
	TempB.SlotIndex = IndexA;

	SlotWidgets[IndexA]->InitSlot(TempB, CachedASC.Get());
	SlotWidgets[IndexB]->InitSlot(TempA, CachedASC.Get());
}

void URActionBarWidget::MoveAbilityToSlot(FGameplayTag AbilityTag, int32 TargetSlotIndex)
{
	if (!SlotWidgets.IsValidIndex(TargetSlotIndex) || !AbilityTag.IsValid())
	{
		return;
	}

	int32 SourceIndex = -1;
	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		if (SlotWidgets[i] && SlotWidgets[i]->GetAbilityTag().MatchesTagExact(AbilityTag))
		{
			SourceIndex = i;
			break;
		}
	}

	if (SourceIndex >= 0 && SourceIndex != TargetSlotIndex)
	{
		SwapSlots(SourceIndex, TargetSlotIndex);
	}
}

TArray<URActionSlotWidget*> URActionBarWidget::GetSlots() const
{
	TArray<URActionSlotWidget*> Result;
	Result.Reserve(SlotWidgets.Num());
	for (const TObjectPtr<URActionSlotWidget>& SlotWidget : SlotWidgets)
	{
		Result.Add(SlotWidget.Get());
	}
	return Result;
}

// ────────────── Internal ──────────────

void URActionBarWidget::CreateEmptySlots()
{
	if (bSlotsCreated || !UGP_Slots || !ActionSlotClass || Columns <= 0)
	{
		return;
	}

	UGP_Slots->ClearChildren();
	SlotWidgets.Empty();

	for (int32 i = 0; i < Columns; ++i)
	{
		URActionSlotWidget* SlotWidget = CreateWidget<URActionSlotWidget>(GetWorld(), ActionSlotClass);
		if (!SlotWidget)
		{
			continue;
		}

		// Initialize as empty
		FActionSlotViewModel EmptyVM;
		EmptyVM.SlotIndex = i;
		EmptyVM.SlotState = EActionSlotState::Empty;
		SlotWidget->InitSlot(EmptyVM, nullptr);

		SlotWidget->OnSlotInteracted.AddDynamic(this, &URActionBarWidget::HandleSlotInteracted);

		if (auto* GridSlot = UGP_Slots->AddChildToUniformGrid(SlotWidget, 0, i))
		{
			GridSlot->SetHorizontalAlignment(HAlign_Fill);
			GridSlot->SetVerticalAlignment(VAlign_Fill);
		}

		SlotWidgets.Add(SlotWidget);
	}

	bSlotsCreated = true;
}

void URActionBarWidget::FillSlotsFromViewModels(const TArray<FActionSlotViewModel>& ViewModels)
{
	if (!bSlotsCreated)
	{
		CreateEmptySlots();
	}

	UAbilitySystemComponent* ASC = CachedASC.Get();

	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		if (!SlotWidgets[i])
		{
			continue;
		}

		if (i < ViewModels.Num())
		{
			// Fill with ability data
			FActionSlotViewModel VM = ViewModels[i];
			VM.SlotIndex = i;
			SlotWidgets[i]->InitSlot(VM, ASC);
		}
		else
		{
			// Empty slot
			SlotWidgets[i]->ClearSlot();
		}
	}
}

void URActionBarWidget::BindAllSlotEvents()
{
	for (URActionSlotWidget* SlotWidget : SlotWidgets)
	{
		if (SlotWidget)
		{
			SlotWidget->OnSlotInteracted.AddDynamic(this, &URActionBarWidget::HandleSlotInteracted);
		}
	}
}

void URActionBarWidget::UnbindAllSlotEvents()
{
	for (TObjectPtr<URActionSlotWidget>& SlotWidget : SlotWidgets)
	{
		if (SlotWidget)
		{
			SlotWidget->OnSlotInteracted.RemoveDynamic(this, &URActionBarWidget::HandleSlotInteracted);
		}
	}
}

void URActionBarWidget::HandleSlotInteracted(int32 SlotIndex, FGameplayTag AbilityTag)
{
	OnSlotClicked.Broadcast(SlotIndex, AbilityTag);
	BP_OnSlotClicked(SlotIndex, AbilityTag);

	if (!AbilityTag.IsValid())
	{
		return;
	}

	// Toggle selection: clicking the same slot deselects
	if (SelectedAbilityTag.MatchesTagExact(AbilityTag))
	{
		ClearSelection();
		return;
	}

	// Previous selection exists — cancel it (user picked a different ability)
	if (SelectedAbilityInstance.IsValid())
	{
		SelectedAbilityInstance->OnAbilityDeselectedFromBar(true);
		SelectedAbilityInstance = nullptr;
	}

	// Select the new ability
	SelectedAbilityTag = AbilityTag;
	SelectedSlotIndex = SlotIndex;

	// Look up bRequiresTarget from ViewModel
	bSelectedRequiresTarget = false;
	if (SlotWidgets.IsValidIndex(SlotIndex) && SlotWidgets[SlotIndex])
	{
		bSelectedRequiresTarget = SlotWidgets[SlotIndex]->GetViewModel().bRequiresTarget;
	}

	OnAbilitySelected.Broadcast(AbilityTag, bSelectedRequiresTarget);

	// Ability instance'ını bul ve selection montage akışını başlat
	SelectedAbilityInstance = FindAbilityInstance(AbilityTag);
	if (SelectedAbilityInstance.IsValid())
	{
		SelectedAbilityInstance->OnAbilitySelectedFromBar();
	}

	// If ability doesn't require a target, execute immediately (SelfCast — no hit needed)
	if (!bSelectedRequiresTarget)
	{
		TryExecuteSelectedAbility(FHitResult{});
	}
}

void URActionBarWidget::ClearSelection(bool bIsCancelled)
{
	if (!SelectedAbilityTag.IsValid())
	{
		return;
	}

	if (SelectedAbilityInstance.IsValid())
	{
		SelectedAbilityInstance->OnAbilityDeselectedFromBar(bIsCancelled);
		SelectedAbilityInstance = nullptr;
	}

	SelectedAbilityTag = FGameplayTag();
	SelectedSlotIndex = -1;
	bSelectedRequiresTarget = false;
	OnAbilityDeselected.Broadcast();
}

ETargetingType URActionBarWidget::GetSelectedTargetingType() const
{
	if (SelectedAbilityInstance.IsValid())
	{
		return SelectedAbilityInstance->GetIndicatorConfig().TargetingType;
	}
	return ETargetingType::SelfCast;
}

bool URActionBarWidget::TryExecuteSelectedAbility(const FHitResult& TargetHit)
{
	if (!SelectedAbilityTag.IsValid())
	{
		return false;
	}

	PendingTargetHit = TargetHit;

	bool bSuccess = false;
	if (SlotWidgets.IsValidIndex(SelectedSlotIndex) && SlotWidgets[SelectedSlotIndex])
	{
		bSuccess = SlotWidgets[SelectedSlotIndex]->TryActivateAbility();
	}

	PendingTargetHit = FHitResult{};
	ClearSelection(false);
	return bSuccess;
}

// ────────────── EventBus Handlers ──────────────

void URActionBarWidget::RefreshForSelectedCharacter()
{
	if (!DisplayTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionBar: DisplayTable is NULL — set it in Blueprint defaults or call SetDisplayTable()."));
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionBar: RefreshForSelectedCharacter — GameInstance is NULL."));
		return;
	}

	auto* PartySub = GI->GetSubsystem<URPartySubsystem>();
	if (!PartySub)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionBar: RefreshForSelectedCharacter — PartySubsystem is NULL."));
		return;
	}

	ARCharacterBase* SelectedChar = PartySub->GetSelectedCharacter();
	if (!SelectedChar)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionBar: RefreshForSelectedCharacter — No selected character."));
		UnbindAttributeCallbacks();
		CachedASC = nullptr;
		CachedCharacter = nullptr;
		ClearAllSlots();
		return;
	}

	UnbindAttributeCallbacks();
	CachedCharacter = SelectedChar;

	// Get ASC from selected character
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(SelectedChar))
	{
		CachedASC = ASI->GetAbilitySystemComponent();
	}
	else
	{
		CachedASC = SelectedChar->FindComponentByClass<UAbilitySystemComponent>();
	}

	if (!CachedASC.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionBar: RefreshForSelectedCharacter — Selected character '%s' has no ASC."),
			*SelectedChar->GetName());
	}

	BindAttributeCallbacks(CachedASC.Get());

	RefreshAllSlots();

	OnTurnCharacterChanged.Broadcast(SelectedChar);
	BP_OnTurnCharacterChanged(SelectedChar);

	// Broadcast initial attribute values so the UI starts in the correct state.
	if (CachedASC.IsValid())
	{
		if (const URCharacterTurnAttributeSet* Attrs = CachedASC->GetSet<URCharacterTurnAttributeSet>())
		{
			const float AP = Attrs->GetActionPoints();
			OnActionPointsChanged.Broadcast(AP);
			BP_OnActionPointsChanged(AP);

			const float Walk = Attrs->GetWalkDistance();
			const float MaxWalk = Attrs->GetMaxWalkDistance();
			OnWalkDistanceChanged.Broadcast(Walk, MaxWalk);
			BP_OnWalkDistanceChanged(Walk, MaxWalk);
		}
	}
}

void URActionBarWidget::OnTurnStarted(const FTurnStarted& Event)
{
	// Turn changed — get the new selected character's abilities
	RefreshForSelectedCharacter();
}

void URActionBarWidget::OnAbilityGranted(const FAbilityGranted& Event)
{
	if (CachedASC.IsValid() && Event.OwnerActor == CachedASC->GetOwner())
	{
		RefreshAllSlots();
	}
}

void URActionBarWidget::OnAbilityRemoved(const FAbilityRemoved& Event)
{
	if (CachedASC.IsValid() && Event.OwnerActor == CachedASC->GetOwner())
	{
		RefreshAllSlots();
	}
}

void URActionBarWidget::OnSelectedCharacterChanged(const FSelectedCharacterChanged& Event)
{
	RefreshForSelectedCharacter();
}

// ────────────── Attribute Callbacks ──────────────

void URActionBarWidget::BindAttributeCallbacks(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}
	APChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(
		URCharacterTurnAttributeSet::GetActionPointsAttribute())
		.AddUObject(this, &URActionBarWidget::OnActionPointsAttributeChanged);

	WalkDistChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(
		URCharacterTurnAttributeSet::GetWalkDistanceAttribute())
		.AddUObject(this, &URActionBarWidget::OnWalkDistanceAttributeChanged);
}

void URActionBarWidget::UnbindAttributeCallbacks()
{
	if (CachedASC.IsValid())
	{
		CachedASC->GetGameplayAttributeValueChangeDelegate(
			URCharacterTurnAttributeSet::GetActionPointsAttribute()).Remove(APChangedHandle);
		CachedASC->GetGameplayAttributeValueChangeDelegate(
			URCharacterTurnAttributeSet::GetWalkDistanceAttribute()).Remove(WalkDistChangedHandle);
	}
	APChangedHandle.Reset();
	WalkDistChangedHandle.Reset();
}

void URActionBarWidget::OnActionPointsAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnActionPointsChanged.Broadcast(Data.NewValue);
	BP_OnActionPointsChanged(Data.NewValue);
}

void URActionBarWidget::OnWalkDistanceAttributeChanged(const FOnAttributeChangeData& Data)
{
	const float MaxWalk = CachedASC.IsValid()
		? CachedASC->GetNumericAttribute(URCharacterTurnAttributeSet::GetMaxWalkDistanceAttribute())
		: 0.f;
	OnWalkDistanceChanged.Broadcast(Data.NewValue, MaxWalk);
	BP_OnWalkDistanceChanged(Data.NewValue, MaxWalk);
}

// ────────────── Attribute Getters ──────────────

float URActionBarWidget::GetCurrentActionPoints() const
{
	if (!CachedASC.IsValid())
	{
		return 0.f;
	}
	const URCharacterTurnAttributeSet* Attrs = CachedASC->GetSet<URCharacterTurnAttributeSet>();
	return Attrs ? Attrs->GetActionPoints() : 0.f;
}

float URActionBarWidget::GetCurrentWalkDistance() const
{
	if (!CachedASC.IsValid())
	{
		return 0.f;
	}
	const URCharacterTurnAttributeSet* Attrs = CachedASC->GetSet<URCharacterTurnAttributeSet>();
	return Attrs ? Attrs->GetWalkDistance() : 0.f;
}

float URActionBarWidget::GetMaxWalkDistance() const
{
	if (!CachedASC.IsValid())
	{
		return 0.f;
	}
	const URCharacterTurnAttributeSet* Attrs = CachedASC->GetSet<URCharacterTurnAttributeSet>();
	return Attrs ? Attrs->GetMaxWalkDistance() : 0.f;
}

// ────────────── WorldView Handlers ──────────────

void URActionBarWidget::OnWorldViewTogglePreBegin(const FWorldViewTogglePreBegin& Event)
{
	OnWorldViewWillChange.Broadcast(Event.FromView, Event.ToView);
	BP_OnWorldViewWillChange(Event.FromView, Event.ToView);
}

void URActionBarWidget::HandleWorldViewChanged(const FWorldViewChanged& Event)
{
	CurrentWorldView = Event.NewView;
	RefreshAllSlots();
	OnWorldViewChanged.Broadcast(Event.NewView);
	BP_OnWorldViewChanged(Event.NewView);
}

void URActionBarWidget::OnWorldViewToggleCancelled(const FWorldViewToggleCancelled& Event)
{
	OnWorldViewChangeCancelled.Broadcast();
	BP_OnWorldViewChangeCancelled();
}

// ────────────── Ability Instance Lookup ──────────────

URGameplayAbilityBase* URActionBarWidget::FindAbilityInstance(FGameplayTag AbilityTag) const
{
	if (!CachedASC.IsValid() || !AbilityTag.IsValid())
	{
		return nullptr;
	}

	for (FGameplayAbilitySpec& Spec : CachedASC->GetActivatableAbilities())
	{
		const UGameplayAbility* AbilityCDO = Spec.Ability;
		if (!AbilityCDO)
		{
			continue;
		}

		// Asset tag'ler üzerinden eşle (BuildActionBarViewModels ile aynı pattern)
		const FGameplayTagContainer& AssetTags = AbilityCDO->GetAssetTags();
		if (AssetTags.HasTagExact(AbilityTag))
		{
			// InstancedPerActor: önce instance'ı dene, yoksa CDO'yu döndür
			URGameplayAbilityBase* Instance = Cast<URGameplayAbilityBase>(Spec.GetPrimaryInstance());
			if (Instance)
			{
				return Instance;
			}
			return Cast<URGameplayAbilityBase>(Spec.Ability.Get());
		}
	}

	return nullptr;
}
