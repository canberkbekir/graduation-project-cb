// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RCharacterBase.h"
#include "Core/RConversions.h"
#include "Character/RCharacterSpec.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Components/RTurnComponent.h"
#include "Components/REquipmentComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DataTable.h"
#include "GAS/Abilities/RGA_Walk.h"
#include "GAS/Effects/RStatusGameplayEffect.h"
#include "GAS/Attributes/RCharacterCoreAttributeSet.h"
#include "GAS/Attributes/RCharacterCombatAttributeSet.h"
#include "GAS/Attributes/RCharacterTurnAttributeSet.h"
#include "GAS/Attributes/RCharacterDebuffAttributeSet.h"
#include "Interaction/RInteractable.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Events/CombatEvents.h"
#include "Subsystems/REventBusSubsystem.h"
#include "Subsystems/RPartySubsystem.h"
#include "Subsystems/RCombatManagerSubsystem.h"
#include "RizzGameplayTags.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Settings/RPathVisualizationConfig.h"
#include "Components/WidgetComponent.h"
#include "UI/Common/RCharacterWorldWidget.h"


// Sets default values
ARCharacterBase::ARCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	TurnComponent = CreateDefaultSubobject<URTurnComponent>(TEXT("TurnComponent"));
	InventoryComponent = CreateDefaultSubobject<URInventoryComponent>(TEXT("InventoryComponent"));
	EquipmentComponent = CreateDefaultSubobject<UREquipmentComponent>(TEXT("EquipmentComponent"));

	CharacterInfoWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("CharacterInfoWidget"));
	CharacterInfoWidgetComponent->SetupAttachment(GetRootComponent());
	CharacterInfoWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	CharacterInfoWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	CharacterInfoWidgetComponent->SetDrawSize(FVector2D(300.f, 200.f));
	CharacterInfoWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PathPreviewSpline"));
	PathSpline->SetupAttachment(GetRootComponent());
	PathSpline->SetMobility(EComponentMobility::Movable);
}

FORCEINLINE UAbilitySystemComponent* ARCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ARCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Preview widget position in editor when offset values are changed.
	// Side offset is shown along the local Y axis as a static approximation
	// (runtime uses camera-relative direction instead).
	if (CharacterInfoWidgetComponent)
	{
		CharacterInfoWidgetComponent->SetRelativeLocation(
			FVector(0.f, InfoWidgetSideOffset, InfoWidgetHeightOffset));
	}

	if (!CharacterRow.DataTable || CharacterRow.RowName.IsNone())
	{
		return;
	}

	const FCharacterDefinitionRow* Def = CharacterRow.DataTable->FindRow<FCharacterDefinitionRow>(
		CharacterRow.RowName, TEXT("ARCharacterBase::OnConstruction"));

	if (Def)
	{
		ApplyVisualsFromDefinition(Def);
	}
}

void ARCharacterBase::BeginPlay()
{
	// Grant default walk ability
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// Link equipment and inventory components
	if (EquipmentComponent && InventoryComponent)
	{
		EquipmentComponent->SetLinkedInventory(InventoryComponent);
	}

	// Initialize from CharacterRow if set
	if (CharacterRow.DataTable && !CharacterRow.RowName.IsNone())
	{
		InitFromDataTable(const_cast<UDataTable*>(CharacterRow.DataTable.Get()), CharacterRow.RowName);
	}
	
	
	Super::BeginPlay();

	if (CharacterInfoWidgetClass)
	{
		CharacterInfoWidgetComponent->SetWidgetClass(CharacterInfoWidgetClass);
		CharacterInfoWidgetComponent->InitWidget();
		if (URCharacterWorldWidget* W = Cast<URCharacterWorldWidget>(CharacterInfoWidgetComponent->GetWidget()))
		{
			W->InitForCharacter(this);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("%s BeginPlay"), *GetName());

	if (UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		const ECollisionResponse VisResp = Cap->GetCollisionResponseToChannel(ECC_Visibility);
		UE_LOG(LogTemp, Warning, TEXT("[Collision] %s capsule profile='%s' Visibility=%s"),
			*GetName(),
			*Cap->GetCollisionProfileName().ToString(),
			VisResp == ECR_Block ? TEXT("Block") : (VisResp == ECR_Overlap ? TEXT("Overlap") : TEXT("Ignore")));
	}
	if (TurnComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s StateTreeComp = %s"),
			*GetName(),
			*GetNameSafe(TurnComponent->StateTreeComponent));

		if (TurnComponent->StateTreeComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s RunStatus at BeginPlay = %d"),
				*GetName(),
				(int32)TurnComponent->StateTreeComponent->GetStateTreeRunStatus());
		}

		// Characters placed in the level with Team=Player auto-register with the party.
		// Spawned player characters are added via GameMode::RestartPlayerAtPlayerStart.
		if (TurnComponent->Team == ERCombatTeam::Player)
		{
			if (UGameInstance* GI = GetGameInstance())
			{
				if (URPartySubsystem* Party = GI->GetSubsystem<URPartySubsystem>())
				{
					Party->AddToParty(this);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s TurnComponent is null at BeginPlay"), *GetName());
	}
}

void ARCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bInfoWidgetBillboard && CharacterInfoWidgetComponent)
	{
		UpdateInfoWidgetTransform(DeltaTime);
	}
}

void ARCharacterBase::UpdateInfoWidgetTransform_Implementation(float DeltaTime)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}

	const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
	const FRotator CamRot = PC->PlayerCameraManager->GetCameraRotation();

	// ── Side offset ─────────────────────────────────────────────────────────
	// Camera's right vector (flattened to horizontal plane)
	FVector CamRight = FRotationMatrix(CamRot).GetScaledAxis(EAxis::Y);
	CamRight.Z = 0.f;
	CamRight.Normalize();

	// Is the character to the right or left of the pawn from the camera's perspective?
	const FVector PawnLoc = PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : CamLoc;
	const float SideSign = FVector::DotProduct(GetActorLocation() - PawnLoc, CamRight) >= 0.f ? 1.f : -1.f;

	// Target world position: side offset + height
	const FVector TargetLoc = GetActorLocation()
		+ CamRight * (SideSign * InfoWidgetSideOffset)
		+ FVector(0.f, 0.f, InfoWidgetHeightOffset);

	const FVector NewLoc = FMath::VInterpTo(
		CharacterInfoWidgetComponent->GetComponentLocation(),
		TargetLoc, DeltaTime, InfoWidgetInterpSpeed);

	// ── Yaw-only billboard ───────────────────────────────────────────────────
	// Widget stays perfectly upright — only rotates horizontally toward the camera.
	// Dropping Z from the direction vector gives yaw-only with pitch=0, roll=0.
	FVector ToCamH = CamLoc - NewLoc;
	ToCamH.Z = 0.f;
	ToCamH.Normalize();
	const FRotator TargetRot(0.f, ToCamH.Rotation().Yaw, 0.f);

	const FRotator NewRot = FMath::RInterpTo(
		CharacterInfoWidgetComponent->GetComponentRotation(),
		TargetRot, DeltaTime, InfoWidgetInterpSpeed);

	CharacterInfoWidgetComponent->SetWorldLocationAndRotation(NewLoc, NewRot);
}

void ARCharacterBase::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	AIController = Cast<AAIController>(Controller);
}

bool ARCharacterBase::MoveToLocation(const FVector& Location)
{
	FreezeWalkPath();
	//UE_LOG(LogTemp, Log, TEXT("MoveToLocation %s"), *Location.ToString());

	if (UAbilitySystemComponent* Asc = GetAbilitySystemComponent())
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Ability.Walk"));
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(Tag);
		Asc->TryActivateAbilitiesByTag(TagContainer);
	}

	return false;
}

void ARCharacterBase::StopMoving()
{
	GetCharacterMovement()->StopMovementImmediately();
}

bool ARCharacterBase::Interact(const AActor& TargetActor)
{
	if (!AbilitySystemComponent)
	{
		return false;
	}

	if (!TargetActor.GetClass()->ImplementsInterface(URInteractable::StaticClass()))
	{
		return false;
	}

	FGameplayEventData EventData;
	EventData.Instigator = this;
	EventData.Target = const_cast<AActor*>(&TargetActor);

	static FGameplayTag InteractEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Interact"));

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		this,
		InteractEventTag,
		EventData
	);

	return true;
}

void ARCharacterBase::InitFromDataTable(UDataTable* DataTable, FName RowName)
{
	if (!DataTable || !AbilitySystemComponent)
	{
		return;
	}

	const FCharacterDefinitionRow* Def = DataTable->FindRow<FCharacterDefinitionRow>(RowName, TEXT("ARCharacterBase::InitFromDataTable"));
	if (!Def)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitFromDataTable: Could not find row '%s' in DataTable '%s'"), *RowName.ToString(), *DataTable->GetName());
		return;
	}

	InitFromDefinition(Def);
}

void ARCharacterBase::InitFromDefinition(const FCharacterDefinitionRow* Def)
{
	if (!Def)
	{
		return;
	}

	// ==================== VISUALS + CHARACTER INFO ====================
	ApplyVisualsFromDefinition(Def);

	// ==================== ASC INITIALIZATION ====================
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// ==================== STARTUP TAGS ====================
	if (!Def->StartupTags.IsEmpty())
	{
		AbilitySystemComponent->AddLooseGameplayTags(Def->StartupTags);
	}

	// ==================== ATTRIBUTE SETS ====================

	// Core Attributes
	CoreAttributeSet = NewObject<URCharacterCoreAttributeSet>(this);
	CoreAttributeSet->InitTech(Def->Tech);
	CoreAttributeSet->InitPhysique(Def->Physique);
	CoreAttributeSet->InitFinesse(Def->Finesse);
	CoreAttributeSet->InitMind(Def->Mind);
	CoreAttributeSet->InitInsight(Def->Insight);
	AbilitySystemComponent->AddAttributeSetSubobject(CoreAttributeSet.Get());

	// Combat Attributes
	CombatAttributeSet = NewObject<URCharacterCombatAttributeSet>(this);
	CombatAttributeSet->InitMaxHealth(Def->MaxHealth);
	CombatAttributeSet->InitHealth(Def->Health);
	CombatAttributeSet->InitKineticShields(Def->KineticShields);
	CombatAttributeSet->InitMaxKineticShields(Def->MaxKineticShields);
	CombatAttributeSet->InitEnergyShields(Def->EnergyShields);
	CombatAttributeSet->InitMaxEnergyShields(Def->MaxEnergyShields);
	CombatAttributeSet->InitFreshTrauma(Def->FreshTrauma);
	CombatAttributeSet->InitOldTrauma(Def->OldTrauma);
	AbilitySystemComponent->AddAttributeSetSubobject(CombatAttributeSet.Get());

	// Turn Attributes
	TurnAttributeSet = NewObject<URCharacterTurnAttributeSet>(this);
	TurnAttributeSet->InitActionPoints(Def->ActionPoints);
	TurnAttributeSet->InitMaxWalkDistance(RConversions::FeetToUU(Def->MaxWalkDistance));
	TurnAttributeSet->InitWalkDistance(RConversions::FeetToUU(Def->MaxWalkDistance));
	AbilitySystemComponent->AddAttributeSetSubobject(TurnAttributeSet.Get());

	// Debuff Attributes — all default to 0, written by status GE modifiers at runtime.
	DebuffAttributeSet = NewObject<URCharacterDebuffAttributeSet>(this);
	AbilitySystemComponent->AddAttributeSetSubobject(DebuffAttributeSet.Get());

	// ==================== ABILITIES ====================
	if (HasAuthority())
	{
		for (const TSubclassOf<UGameplayAbility>& AbilityClass : Def->StartingAbilities)
		{
			if (AbilityClass)
			{
				FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
				AbilitySystemComponent->GiveAbility(AbilitySpec);

				// Broadcast so UI (ActionBar etc.) can react
				if (auto* GI = GetGameInstance())
				{
					if (auto* Bus = GI->GetSubsystem<UREventBusSubsystem>())
					{
						const UGameplayAbility* CDO = AbilityClass.GetDefaultObject();
						for (const FGameplayTag& Tag : CDO->GetAssetTags())
						{
							FAbilityGranted Event;
							Event.AbilityTag = Tag;
							Event.OwnerActor = this;
							Bus->Publish(Event);
						}
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Character '%s' initialized from DataTable definition"), *GetName());
}

void ARCharacterBase::ShowInfoWidget()
{
	if (URCharacterWorldWidget* W = Cast<URCharacterWorldWidget>(CharacterInfoWidgetComponent->GetWidget()))
	{
		W->OnCursorEnter();
	}
}

void ARCharacterBase::HideInfoWidget()
{
	if (URCharacterWorldWidget* W = Cast<URCharacterWorldWidget>(CharacterInfoWidgetComponent->GetWidget()))
	{
		W->OnCursorLeave();
	}
}

void ARCharacterBase::ApplyVisualsFromDefinition(const FCharacterDefinitionRow* Def)
{
	if (!Def)
	{
		return;
	}

	DisplayName = Def->DisplayName;
	Portrait = Def->Portrait.LoadSynchronous();

	USkeletalMeshComponent* MeshComp = GetMesh();

	if (USkeletalMesh* LoadedMesh = Def->SkeletalMesh.LoadSynchronous())
	{
		MeshComp->SetSkeletalMesh(LoadedMesh);
		MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);

		if (UClass* LoadedAnimClass = Def->AnimClass.LoadSynchronous())
		{
			MeshComp->SetAnimInstanceClass(LoadedAnimClass);
		}

		MeshComp->InitAnim(true);
		MeshComp->bPauseAnims = false;
		MeshComp->GlobalAnimRateScale = 1.f;
	}
	else if (UClass* LoadedAnimClass = Def->AnimClass.LoadSynchronous())
	{
		MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		MeshComp->SetAnimInstanceClass(LoadedAnimClass);
		MeshComp->InitAnim(true);
	}
}

// ============================================================
// Death System
// ============================================================

void ARCharacterBase::HandleDeath(ARCharacterBase* Killer)
{
	if (bIsDead) return;
	bIsDead = true;

	// 1. Add State.Dead tag so abilities with BlockAbilitiesWithTag=State.Dead are blocked.
	if (AbilitySystemComponent)
	{
		FGameplayTagContainer DeadTag;
		DeadTag.AddTag(TAG_State_Dead);
		AbilitySystemComponent->AddLooseGameplayTags(DeadTag);
		AbilitySystemComponent->CancelAllAbilities();
	}

	// 2. Stop movement so the character doesn't path-follow or slide after dying.
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	// 3. Disable capsule collision so other pawns walk through the corpse.
	if (UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		Cap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 4. Hide the floating info widget — dead characters don't need a health bar or nameplate.
	if (CharacterInfoWidgetComponent)
	{
		CharacterInfoWidgetComponent->SetVisibility(false);
	}

	// 5. Remove from turn order before delegates fire so UI/AI see the correct order immediately.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (URCombatManagerSubsystem* CombatMgr = GI->GetSubsystem<URCombatManagerSubsystem>())
		{
			CombatMgr->RemoveFromCombat(TurnComponent);
		}
	}

	// 6. Blueprint hook — VFX, SFX, camera shake, particle effects, etc.
	OnDeath(Killer);

	// 7. Notify all subscribers (UI kill feed, AI awareness, quest system, etc.).
	OnCharacterDied.Broadcast(this, Killer);

	// 8. Play death animation, or call FinishDeath immediately if no montage is assigned.
	PlayDeathMontage();
}

void ARCharacterBase::OnDeath_Implementation(ARCharacterBase* Killer)
{
	// Default no-op. Override in Blueprint per character.
}

void ARCharacterBase::PlayDeathMontage()
{
	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInst && DeathMontage)
	{
		const float Duration = PlayAnimMontage(DeathMontage);
		if (Duration > 0.f)
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ARCharacterBase::FinishDeath_OnMontageEnded);
			AnimInst->Montage_SetEndDelegate(EndDelegate, DeathMontage);
			return;
		}
	}
	// No montage assigned or montage failed to play — finish immediately.
	FinishDeath();
}

void ARCharacterBase::FinishDeath_OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	FinishDeath();
}

void ARCharacterBase::FinishDeath()
{
	if (!bIsDead || bDeathFinished) return;
	bDeathFinished = true;

	if (bEnableRagdollOnDeath)
	{
		if (RagdollDelay > 0.f)
		{
			GetWorldTimerManager().SetTimer(DeathTimerHandle, this,
				&ARCharacterBase::StartRagdoll, RagdollDelay, false);
		}
		else
		{
			StartRagdoll();
		}
	}
	else
	{
		// Freeze the skeleton in the last death pose.
		// Without this the AnimBP resumes and can slide the character back to idle.
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			MeshComp->bPauseAnims = true;
		}
	}

	if (bDestroyOnDeath)
	{
		FTimerHandle DestroyHandle;
		GetWorldTimerManager().SetTimer(DestroyHandle,
			[this]() { Destroy(); }, FMath::Max(DestroyDelay, 0.001f), false);
	}
}

void ARCharacterBase::StartRagdoll()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp) return;

	MeshComp->bPauseAnims = true;
	MeshComp->SetAllBodiesSimulatePhysics(true);
	MeshComp->SetSimulatePhysics(true);
	MeshComp->WakeAllRigidBodies();
	MeshComp->bBlendPhysics = true;
	MeshComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

// ============================================================
// Corpse Loot
// ============================================================

bool ARCharacterBase::TakeAllCorpseLoot(AActor* InteractingActor)
{
	if (!InteractingActor || !bIsDead || !bLootableOnDeath || bCorpseLootTaken)
	{
		return false;
	}

	URInventoryComponent* Inventory = InteractingActor->FindComponentByClass<URInventoryComponent>();
	if (!Inventory) return false;

	bool bAnyTaken = false;
	for (int32 i = DeathLoot.Entries.Num() - 1; i >= 0; --i)
	{
		const FLootEntry& Entry = DeathLoot.Entries[i];
		if (Entry.Item && Entry.Quantity > 0 && Inventory->TryAddItem(Entry.Item, Entry.Quantity))
		{
			DeathLoot.Entries.RemoveAt(i);
			bAnyTaken = true;
		}
	}

	if (bAnyTaken && DeathLoot.Entries.IsEmpty())
	{
		bCorpseLootTaken = true;
		BP_OnCorpseLootEmptied(InteractingActor);
	}

	return bAnyTaken;
}

// ============================================================
// IRInteractable — corpse loot (only active when dead + lootable)
// ============================================================

bool ARCharacterBase::CanInteract_Implementation(APawn* InteractingPawn)
{
	if (!bIsDead || !bLootableOnDeath || bCorpseLootTaken || !InteractingPawn) return false;

	const float DistSq = FVector::DistSquared(InteractingPawn->GetActorLocation(), GetActorLocation());
	return DistSq <= FMath::Square(CorpseLootRadius);
}

void ARCharacterBase::Interact_Implementation(APawn* InteractingPawn)
{
	if (!InteractingPawn) return;
	BP_OnCorpseLootOpened(InteractingPawn);
}

#define LOCTEXT_NAMESPACE "ARCharacterBase"

FText ARCharacterBase::GetInteractionName_Implementation()
{
	return DisplayName.IsEmpty()
		? LOCTEXT("CorpseNameDefault", "Body")
		: DisplayName;
}

FText ARCharacterBase::GetInteractionAction_Implementation()
{
	return bCorpseLootTaken
		? LOCTEXT("CorpseActionEmpty", "Inspect")
		: LOCTEXT("CorpseActionLoot", "Loot");
}

#undef LOCTEXT_NAMESPACE

// ============================================================
// Path Preview
// ============================================================

static TArray<FVector> CapPointsAtDistance(const TArray<FVector>& Points, float MaxDist)
{
	TArray<FVector> Result;
	if (Points.IsEmpty() || MaxDist <= 0.f)
		return Result;
	Result.Add(Points[0]);
	float AccumDist = 0.f;
	for (int32 i = 1; i < Points.Num(); ++i)
	{
		const float SegLen = FVector::Dist(Points[i - 1], Points[i]);
		if (AccumDist + SegLen >= MaxDist)
		{
			const float Remaining = MaxDist - AccumDist;
			Result.Add(Points[i - 1] + (Points[i] - Points[i - 1]).GetSafeNormal() * Remaining);
			break;
		}
		AccumDist += SegLen;
		Result.Add(Points[i]);
	}
	return Result;
}

template<typename TComp>
static void ResizeComponentPool(TArray<TObjectPtr<TComp>>& Pool, int32 Count, USceneComponent* Parent, AActor* Owner)
{
	while (Pool.Num() < Count)
	{
		TComp* Comp = NewObject<TComp>(Owner);
		Comp->SetupAttachment(Parent);
		Comp->SetMobility(EComponentMobility::Movable);
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Comp->SetVisibility(false);
		Comp->RegisterComponent();
		Pool.Add(Comp);
	}
	for (int32 i = Count; i < Pool.Num(); ++i)
		if (Pool[i]) Pool[i]->SetVisibility(false);
}

void ARCharacterBase::ShowPathPreview(const TArray<FVector>& Points, URPathVisualizationConfig* Config)
{
	if (!Config || Points.Num() < 2 || Config->PathSegmentMesh.IsNull())
	{
		ClearPathPreview();
		return;
	}

	// Cap path at remaining walk distance for the current turn
	TArray<FVector> WorkPoints;
	if (TurnAttributeSet)
	{
		WorkPoints = CapPointsAtDistance(Points, TurnAttributeSet->GetWalkDistance());
	}
	else
	{
		WorkPoints = Points;
	}

	if (WorkPoints.Num() < 2)
	{
		ClearPathPreview();
		return;
	}

	if (WorkPoints == LastPreviewPoints)
		return;
	LastPreviewPoints = WorkPoints;

	UStaticMesh*        SegMesh = Config->PathSegmentMesh.LoadSynchronous();
	UMaterialInterface* SegMat  = Config->PathMaterial.IsNull()
	                              ? nullptr : Config->PathMaterial.LoadSynchronous();
	const ESplineMeshAxis::Type Axis = static_cast<ESplineMeshAxis::Type>(
		FMath::Clamp(Config->ForwardAxis, 0, 2));

	PathSpline->ClearSplinePoints(false);
	for (const FVector& Pt : WorkPoints)
		PathSpline->AddSplineWorldPoint(Pt);
	PathSpline->UpdateSpline();

	const int32 SegCount = WorkPoints.Num() - 1;
	ResizeComponentPool(PathSegments, SegCount, PathSpline, this);

	for (int32 i = 0; i < SegCount; ++i)
	{
		USplineMeshComponent* Seg = PathSegments[i];
		FVector StartPos, StartTan, EndPos, EndTan;
		PathSpline->GetLocationAndTangentAtSplinePoint(i,     StartPos, StartTan, ESplineCoordinateSpace::Local);
		PathSpline->GetLocationAndTangentAtSplinePoint(i + 1, EndPos,   EndTan,   ESplineCoordinateSpace::Local);
		Seg->SetStartAndEnd(StartPos, StartTan, EndPos, EndTan, false);
		Seg->SetForwardAxis(Axis, false);
		Seg->SetStartScale(Config->MeshScale, false);
		Seg->SetEndScale(Config->MeshScale, false);
		Seg->SetStartOffset(Config->MeshOffset, false);
		Seg->SetEndOffset(Config->MeshOffset, true);
		Seg->SetStaticMesh(SegMesh);
		if (SegMat) Seg->SetMaterial(0, SegMat);
		Seg->SetVisibility(true);
	}

	UStaticMesh*        WpMesh = (Config->bShowWaypoints && !Config->WaypointMesh.IsNull())
	                             ? Config->WaypointMesh.LoadSynchronous() : nullptr;
	UMaterialInterface* WpMat  = (WpMesh && !Config->WaypointMaterial.IsNull())
	                             ? Config->WaypointMaterial.LoadSynchronous() : nullptr;
	const int32 Last = WorkPoints.Num() - 1;

	ResizeComponentPool(PathWaypoints, WorkPoints.Num(), PathSpline, this);
	for (int32 i = 0; i < WorkPoints.Num(); ++i)
	{
		UStaticMeshComponent* Wp    = PathWaypoints[i];
		UStaticMesh*        WpSlotMesh = WpMesh;
		UMaterialInterface* WpSlotMat  = WpMat;
		if (i == 0 && Config->bShowStartPoint && !Config->StartPointMesh.IsNull())
		{
			WpSlotMesh = Config->StartPointMesh.LoadSynchronous();
			WpSlotMat  = Config->StartPointMaterial.IsNull() ? nullptr : Config->StartPointMaterial.LoadSynchronous();
		}
		else if (i == Last && Config->bShowEndPoint && !Config->EndPointMesh.IsNull())
		{
			WpSlotMesh = Config->EndPointMesh.LoadSynchronous();
			WpSlotMat  = Config->EndPointMaterial.IsNull() ? nullptr : Config->EndPointMaterial.LoadSynchronous();
		}
		if (WpSlotMesh)
		{
			Wp->SetWorldLocation(WorkPoints[i]);
			Wp->SetStaticMesh(WpSlotMesh);
			if (WpSlotMat) Wp->SetMaterial(0, WpSlotMat);
			Wp->SetVisibility(true);
		}
		else
		{
			Wp->SetVisibility(false);
		}
	}

	bPathPreviewVisible = true;
}

void ARCharacterBase::FreezeWalkPath()
{
	if (bPathPreviewVisible)
		bWalkPathFrozen = true;
}

void ARCharacterBase::UpdateWalkPathShrink()
{
	if (!bWalkPathFrozen || !PathSpline || PathSegments.IsEmpty())
		return;

	const float InputKey = PathSpline->FindInputKeyClosestToWorldLocation(GetActorLocation());
	const float CharDist = PathSpline->GetDistanceAlongSplineAtSplineInputKey(InputKey);

	const int32 NumSegs = PathSegments.Num();
	for (int32 i = 0; i < NumSegs; ++i)
	{
		USplineMeshComponent* Seg = PathSegments[i];
		if (!Seg) continue;

		const float SegStartDist = PathSpline->GetDistanceAlongSplineAtSplinePoint(i);
		const float SegEndDist   = PathSpline->GetDistanceAlongSplineAtSplinePoint(i + 1);

		if (CharDist >= SegEndDist)
		{
			Seg->SetVisibility(false);
		}
		else if (CharDist > SegStartDist)
		{
			const FVector NewStart    = PathSpline->GetLocationAtDistanceAlongSpline(CharDist, ESplineCoordinateSpace::Local);
			const FVector NewStartTan = PathSpline->GetTangentAtDistanceAlongSpline(CharDist, ESplineCoordinateSpace::Local);
			FVector EndPos, EndTan;
			PathSpline->GetLocationAndTangentAtSplinePoint(i + 1, EndPos, EndTan, ESplineCoordinateSpace::Local);
			Seg->SetStartAndEnd(NewStart, NewStartTan, EndPos, EndTan, true);
			Seg->SetVisibility(true);
		}
	}

	// Hide waypoints the character has already passed (keep destination visible)
	const int32 NumWaypoints = PathWaypoints.Num();
	for (int32 i = 0; i < NumWaypoints - 1; ++i)
	{
		if (!PathWaypoints[i]) continue;
		const float WpDist = PathSpline->GetDistanceAlongSplineAtSplinePoint(i);
		PathWaypoints[i]->SetVisibility(CharDist < WpDist);
	}
}

void ARCharacterBase::ClearPathPreview()
{
	bWalkPathFrozen = false;
	if (!bPathPreviewVisible)
		return;
	bPathPreviewVisible = false;
	LastPreviewPoints.Reset();

	for (USplineMeshComponent* Seg : PathSegments)
		if (Seg) Seg->SetVisibility(false);
	for (UStaticMeshComponent* Wp : PathWaypoints)
		if (Wp) Wp->SetVisibility(false);
	if (PathSpline) PathSpline->ClearSplinePoints();
}

float ARCharacterBase::GetInteractionRadius_Implementation()
{
	return CorpseLootRadius;
}

FVector ARCharacterBase::GetInteractionLocation_Implementation()
{
	return GetActorLocation();
}
