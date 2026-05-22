#include "Core/RPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/EngineTypes.h"
#include "Core/RClickResolver.h"
#include "Core/RCharacterBase.h"
#include "Components/RSelectionComponent.h"
#include "Interaction/ISelectable.h"
#include "Subsystems/RPartySubsystem.h"
#include "Subsystems/RCombatManagerSubsystem.h"
#include "Subsystems/REventBusSubsystem.h"
#include "UI/Combat/RActionBarWidget.h"
#include "Events/CombatEvents.h"
#include "Components/RTurnComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogRPlayerController, Log, All);

// Tries a character-object trace first (works regardless of ECC_Visibility settings on characters),
// then falls back to the provided world hit. Returns the best hit.
static FHitResult ResolveBestHit(APlayerController* PC,
	const TArray<TEnumAsByte<EObjectTypeQuery>>& CharacterTypes,
	const FHitResult& WorldHit)
{
	if (CharacterTypes.Num() > 0)
	{
		FHitResult CharHit;
		PC->GetHitResultUnderCursorForObjects(CharacterTypes, false, CharHit);
		if (Cast<ARCharacterBase>(CharHit.GetActor()))
		{
			return CharHit;
		}
	}
	return WorldHit;
}

ARPlayerController::ARPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	PrimaryActorTick.bCanEverTick = true;

	// Default: detect characters via the Pawn object type.
	CharacterObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void ARPlayerController::BeginPlay()
{
	Super::BeginPlay(); // Adds BaseMappingContext (priority 0) + ExplorationMappingContext (priority 1)

	ClickResolver = NewObject<URClickResolver>(this);
	ClickResolver->Initialize(this);
	if (ActionBarWidget.IsValid())
	{
		ClickResolver->SetActionBar(ActionBarWidget.Get());
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UREventBusSubsystem* Bus = GI->GetSubsystem<UREventBusSubsystem>())
		{
			Bus->Subscribe<FCombatStarted>(this, &ARPlayerController::OnCombatStarted);
			Bus->Subscribe<FCombatEnded>(this, &ARPlayerController::OnCombatEnded);
		}
	}
}

void ARPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent(); // Binds camera + exploration actions

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC)
	{
		UE_LOG(LogRPlayerController, Error, TEXT("ARPlayerController requires an Enhanced Input Component."));
		return;
	}

	if (ClickAction)
	{
		EIC->BindAction(ClickAction, ETriggerEvent::Started, this, &ARPlayerController::OnClick);
	}

	if (SecondaryClickAction)
	{
		EIC->BindAction(SecondaryClickAction, ETriggerEvent::Started, this, &ARPlayerController::OnSecondaryClick);
	}
}

void ARPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateHover();
}

void ARPlayerController::SetActionBar(URActionBarWidget* InActionBar)
{
	ActionBarWidget = InActionBar;
	if (ClickResolver)
	{
		ClickResolver->SetActionBar(InActionBar);
	}
}

// ---------------------------------------------------------------------------
// Input handlers
// ---------------------------------------------------------------------------

void ARPlayerController::OnClick()
{
	FHitResult VisHit;
	GetHitResultUnderCursor(WorldTraceChannel, true, VisHit);

	const FHitResult Hit = ResolveBestHit(this, CharacterObjectTypes, VisHit);
	if (!Hit.bBlockingHit)
	{
		return;
	}

	if (ClickResolver)
	{
		ClickResolver->Resolve(Hit);
	}
}

void ARPlayerController::OnSecondaryClick()
{
	if (ActionBarWidget.IsValid() && ActionBarWidget->HasSelection())
	{
		ActionBarWidget->ClearSelection(true); // true = user cancelled
		return;
	}

	// Deselect current party selection visual
	if (UGameInstance* GI = GetGameInstance())
	{
		if (URPartySubsystem* Party = GI->GetSubsystem<URPartySubsystem>())
		{
			if (ARCharacterBase* Current = Party->GetSelectedCharacter())
			{
				if (URSelectionComponent* SelComp = Current->FindComponentByClass<URSelectionComponent>())
				{
					SelComp->SetVisualState(ESelectionVisualState::None);
				}
				if (Current->Implements<URSelectable>())
				{
					IRSelectable::Execute_OnDeselected(Current);
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Hover
// ---------------------------------------------------------------------------

void ARPlayerController::UpdateHover()
{
	FHitResult VisHit;
	GetHitResultUnderCursor(WorldTraceChannel, true, VisHit);

	const FHitResult Hit = ResolveBestHit(this, CharacterObjectTypes, VisHit);
	const bool bHit = Hit.bBlockingHit;
	AActor* RawHit = bHit ? Hit.GetActor() : nullptr;

	// Update cursor icon via resolver
	if (ClickResolver)
	{
		CurrentMouseCursor = bHit ? ClickResolver->GetCursorFor(Hit) : EMouseCursor::Default;
	}

	// Drive character info widget visibility
	// Hide uses a short delay to prevent flickering when the cursor briefly
	// leaves the character capsule (e.g. hits the floor tile at their feet).
	ARCharacterBase* NewHoveredChar = Cast<ARCharacterBase>(RawHit);
	if (NewHoveredChar != HoveredCharacter.Get())
	{
		if (NewHoveredChar)
		{
			// Entering a character — cancel any pending hide and show immediately.
			GetWorldTimerManager().ClearTimer(CharacterHideTimerHandle);
			if (ARCharacterBase* Prev = HoveredCharacter.Get())
			{
				Prev->HideInfoWidget();
			}
			HoveredCharacter = NewHoveredChar;
			NewHoveredChar->ShowInfoWidget();
		}
		else
		{
			// Leaving a character — delay the hide so brief cursor wobble doesn't close the widget.
			PendingHideCharacter = HoveredCharacter;
			HoveredCharacter = nullptr;
			GetWorldTimerManager().SetTimer(CharacterHideTimerHandle, [this]()
			{
				if (ARCharacterBase* Prev = PendingHideCharacter.Get())
				{
					Prev->HideInfoWidget();
				}
				PendingHideCharacter = nullptr;
			}, 0.1f, false);
		}
	}

	// Diagnostic: log every time the raw cursor hit changes (fires only on actor change, not every frame).
	if (bEnableHoverLogs)
	{
		static TWeakObjectPtr<AActor> LastRawHit;
		if (RawHit != LastRawHit.Get())
		{
			LastRawHit = RawHit;
			const bool bIsCharacter = Cast<ARCharacterBase>(RawHit) != nullptr;
			UE_LOG(LogRPlayerController, Log, TEXT("[Hover] Cursor hit changed → %s (%s)%s"),
				RawHit ? *RawHit->GetName() : TEXT("None"),
				RawHit ? *RawHit->GetClass()->GetName() : TEXT("None"),
				bIsCharacter ? TEXT(" [CHARACTER]") : TEXT(""));

			if (IsInCombat())
			{
				if (const ARCharacterBase* Char = Cast<ARCharacterBase>(RawHit))
				{
					if (const URTurnComponent* TurnComp = Char->GetTurnComponent())
					{
						switch (TurnComp->Team)
						{
						case ERCombatTeam::Player:
							UE_LOG(LogRPlayerController, Log, TEXT("[Hover][Combat] Ally: %s"), *Char->GetName());
							if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
								FString::Printf(TEXT("[Hover][Combat] Ally: %s"), *Char->GetName()));
							break;
						case ERCombatTeam::Enemy:
							UE_LOG(LogRPlayerController, Warning, TEXT("[Hover][Combat] Enemy: %s"), *Char->GetName());
							if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
								FString::Printf(TEXT("[Hover][Combat] Enemy: %s"), *Char->GetName()));
							break;
						case ERCombatTeam::Neutral:
							UE_LOG(LogRPlayerController, Log, TEXT("[Hover][Combat] Neutral: %s"), *Char->GetName());
							if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
								FString::Printf(TEXT("[Hover][Combat] Neutral: %s"), *Char->GetName()));
							break;
						}
					}
				}
			}
		}
	}

	// --- Ability target hover ---
	// Independent of the selection hover below. Fires whenever the cursor enters or
	// leaves a character while an ability is selected, even if HoveredActor hasn't changed.
	{
		AActor* NewAbilityTarget = nullptr;
		if (ActionBarWidget.IsValid() && ActionBarWidget->HasSelection())
		{
			if (Cast<ARCharacterBase>(RawHit))
			{
				NewAbilityTarget = RawHit;
			}
			else
			{
				NewAbilityTarget = nullptr;
			}
		}

		if (NewAbilityTarget != AbilityHoveredTarget.Get())
		{
			if (AActor* Prev = AbilityHoveredTarget.Get())
			{
				if (bEnableHoverLogs)
					UE_LOG(LogRPlayerController, Log, TEXT("[Hover] AbilityTarget LEAVE: %s"), *Prev->GetName());
				OnAbilityTargetHoverEnd(Prev);
			}
			AbilityHoveredTarget = NewAbilityTarget;
			if (NewAbilityTarget)
			{
				if (bEnableHoverLogs)
					UE_LOG(LogRPlayerController, Log, TEXT("[Hover] AbilityTarget ENTER: %s"), *NewAbilityTarget->GetName());
				OnAbilityTargetHoverBegin(NewAbilityTarget);
			}
		}
	}

	// --- Selection hover (actors with SelectionComponent or ISelectable) ---
	AActor* NewHovered = RawHit;
	URSelectionComponent* NewHoveredSelComp = nullptr;
	if (NewHovered)
	{
		NewHoveredSelComp = NewHovered->FindComponentByClass<URSelectionComponent>();
		const bool bHasInterface = NewHovered->Implements<URSelectable>();
		if (!NewHoveredSelComp && !bHasInterface)
		{
			NewHovered = nullptr;
		}
	}

	if (NewHovered == HoveredActor.Get())
	{
		return;
	}

	// Clear previous selection hover
	if (AActor* PrevHovered = HoveredActor.Get())
	{
		if (PrevHovered->Implements<URSelectable>())
		{
			IRSelectable::Execute_OnHoverEnd(PrevHovered);
		}
		if (URSelectionComponent* SelComp = PrevHovered->FindComponentByClass<URSelectionComponent>())
		{
			if (SelComp->GetVisualState() == ESelectionVisualState::Hovered)
			{
				SelComp->SetVisualState(ESelectionVisualState::None);
			}
		}
	}

	HoveredActor = NewHovered;

	if (NewHovered)
	{
		if (NewHovered->Implements<URSelectable>())
		{
			IRSelectable::Execute_OnHoverBegin(NewHovered);
		}
		if (NewHoveredSelComp && NewHoveredSelComp->GetVisualState() == ESelectionVisualState::None)
		{
			NewHoveredSelComp->SetVisualState(ESelectionVisualState::Hovered);
		}
	}
}

// ---------------------------------------------------------------------------
// Combat context swap
// ---------------------------------------------------------------------------

void ARPlayerController::OnCombatStarted(const FCombatStarted& Event)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem)
	{
		return;
	}

	if (ExplorationMappingContext)
	{
		Subsystem->RemoveMappingContext(ExplorationMappingContext);
	}
	if (CombatMappingContext)
	{
		Subsystem->AddMappingContext(CombatMappingContext, 1);
	}
}

void ARPlayerController::OnCombatEnded(const FCombatEnded& Event)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem)
	{
		return;
	}

	if (CombatMappingContext)
	{
		Subsystem->RemoveMappingContext(CombatMappingContext);
	}
	if (ExplorationMappingContext)
	{
		Subsystem->AddMappingContext(ExplorationMappingContext, 1);
	}
}

// ---------------------------------------------------------------------------
// State queries
// ---------------------------------------------------------------------------

bool ARPlayerController::IsInCombat() const
{
	const URCombatManagerSubsystem* Combat = GetGameInstance()->GetSubsystem<URCombatManagerSubsystem>();
	if (!Combat)
	{
		return false;
	}
	return Combat->Combatants.Num() > 0 &&
		   Combat->CurrentCombatantIndex < static_cast<uint8>(Combat->Combatants.Num());
}

bool ARPlayerController::IsPlayerTurn() const
{
	const URCombatManagerSubsystem* Combat = GetGameInstance()->GetSubsystem<URCombatManagerSubsystem>();
	const URPartySubsystem* Party = GetGameInstance()->GetSubsystem<URPartySubsystem>();

	if (!Combat || !Party || !IsInCombat())
	{
		return false;
	}

	const URTurnComponent* ActiveTurnComp = Combat->Combatants[Combat->CurrentCombatantIndex];
	if (!ActiveTurnComp)
	{
		return false;
	}

	const ARCharacterBase* ActiveCharacter = Cast<ARCharacterBase>(ActiveTurnComp->GetOwner());
	return ActiveCharacter && const_cast<URPartySubsystem*>(Party)->IsInParty(const_cast<ARCharacterBase*>(ActiveCharacter));
}
