// ==== RCharacterBase.h ====
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Engine/DataTable.h"
#include "Components/RInventoryComponent.h"
#include "Items/LootType.h"
#include "Interaction/RInteractable.h"
#include "RCharacterBase.generated.h"

class UAbilitySystemComponent;
class URTurnComponent;
class UREquipmentComponent;
class AAIController;
class UWidgetComponent;
class URCharacterWorldWidget;
class URCharacterCoreAttributeSet;
class URCharacterCombatAttributeSet;
class URCharacterTurnAttributeSet;
class URCharacterDebuffAttributeSet;
class URPathVisualizationConfig;
class USplineComponent;
class USplineMeshComponent;
class UStaticMeshComponent;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterDied, ARCharacterBase*, DeadCharacter, ARCharacterBase*, Killer);

UCLASS()
class RIZZGAME_API ARCharacterBase : public ACharacter, public IAbilitySystemInterface, public IRInteractable
{
	GENERATED_BODY()

public:
	ARCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	AAIController* GetAIController() const { return AIController; }

	// === Movement ===
	bool MoveToLocation(const FVector& Location);
	void StopMoving();

	// === Path preview (driven by PlayerController each tick while InTurn) ===
	void ShowPathPreview(const TArray<FVector>& Points, URPathVisualizationConfig* Config);
	void ClearPathPreview();

	/** Called when the character starts moving — freezes the preview so it shrinks as the character walks. */
	void FreezeWalkPath();

	/** Called each tick while walk is in progress — trims the frozen path from the character's current position. */
	void UpdateWalkPathShrink();

	bool IsWalkPathFrozen() const { return bWalkPathFrozen; }

	// === Interaction ===
	bool Interact(const AActor& TargetActor);

	// === Death — Events ===

	/** Broadcast when this character dies. Carries both the dead character and the killer (null for environmental deaths). */
	UPROPERTY(BlueprintAssignable, Category="Rizz|Death")
	FOnCharacterDied OnCharacterDied;

	/** Returns true once HandleDeath has been called. Safe to call every frame. */
	UFUNCTION(BlueprintPure, Category="Rizz|Death")
	bool IsDead() const { return bIsDead; }

	/**
	 * Called automatically after the death montage ends (via the montage end delegate).
	 * Also BlueprintCallable so a BP AnimNotify can trigger it directly.
	 */
	UFUNCTION(BlueprintCallable, Category="Rizz|Death")
	void FinishDeath();

	/**
	 * Override in Blueprint to play VFX, SFX, or show UI on this character's death.
	 * Killer is null for environmental/DOT deaths. Default C++ impl is a no-op.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Rizz|Death")
	void OnDeath(ARCharacterBase* Killer);
	virtual void OnDeath_Implementation(ARCharacterBase* Killer);

	/**
	 * Primary death entry point — called from PostGameplayEffectExecute when Health == 0.
	 * Idempotent: subsequent calls while bIsDead == true are silent no-ops.
	 */
	void HandleDeath(ARCharacterBase* Killer);

	// === Death — Loot API ===

	/** Returns the loot set on this corpse. Only populated if bLootableOnDeath is true. */
	UFUNCTION(BlueprintCallable, Category="Rizz|Death|Loot")
	const FLootSet& GetDeathLoot() const { return DeathLoot; }

	/** Moves all remaining death loot into the interacting actor's inventory. */
	UFUNCTION(BlueprintCallable, Category="Rizz|Death|Loot")
	bool TakeAllCorpseLoot(AActor* InteractingActor);

	/** True when all DeathLoot entries have been taken. */
	UFUNCTION(BlueprintPure, Category="Rizz|Death|Loot")
	bool IsCorpseLootEmpty() const { return DeathLoot.Entries.IsEmpty(); }

	// === World UI ===
	/** The floating info widget component above this character's head. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rizz|UI")
	TObjectPtr<UWidgetComponent> CharacterInfoWidgetComponent;

	/** Assign a BP child of URCharacterWorldWidget to activate the world-space panel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|UI")
	TSubclassOf<URCharacterWorldWidget> CharacterInfoWidgetClass;

	/** Call from PlayerController's OnActorBeginCursorOver to show the info panel. */
	UFUNCTION(BlueprintCallable, Category="Rizz|UI")
	void ShowInfoWidget();

	/** Call from PlayerController's OnActorEndCursorOver to hide the info panel. */
	UFUNCTION(BlueprintCallable, Category="Rizz|UI")
	void HideInfoWidget();

	/** When true, the info widget faces the camera and repositions itself every frame. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|UI")
	bool bInfoWidgetBillboard = true;

	/** Interp speed for both rotation and position. Higher = snappier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|UI", meta=(ClampMin="0.1", EditCondition="bInfoWidgetBillboard"))
	float InfoWidgetInterpSpeed = 10.f;

	/** How far left/right of the character the widget sits (world units). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|UI", meta=(EditCondition="bInfoWidgetBillboard"))
	float InfoWidgetSideOffset = 80.f;

	/** Height above the character's root. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|UI")
	float InfoWidgetHeightOffset = 130.f;

	/**
	 * Override in BP to fully customize positioning and rotation.
	 * Default: Z-locked billboard + side offset based on pawn-relative camera direction.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Rizz|UI")
	void UpdateInfoWidgetTransform(float DeltaTime);
	virtual void UpdateInfoWidgetTransform_Implementation(float DeltaTime);

	// === Components ===
	TObjectPtr<URTurnComponent> GetTurnComponent() const { return TurnComponent; }
	TObjectPtr<URInventoryComponent> GetInventoryComponent() const { return InventoryComponent; }
	TObjectPtr<UREquipmentComponent> GetEquipmentComponent() const { return EquipmentComponent; }

	// === Attribute Sets ===
	UFUNCTION(BlueprintCallable, Category="Rizz|Attributes")
	URCharacterCoreAttributeSet* GetCoreAttributes() const { return CoreAttributeSet; }

	UFUNCTION(BlueprintCallable, Category="Rizz|Attributes")
	URCharacterCombatAttributeSet* GetCombatAttributes() const { return CombatAttributeSet; }

	UFUNCTION(BlueprintCallable, Category="Rizz|Attributes")
	URCharacterTurnAttributeSet* GetTurnAttributes() const { return TurnAttributeSet; }

	// === Character Info (from DataTable) ===
	UFUNCTION(BlueprintCallable, Category="Rizz|Info")
	FText GetDisplayName() const { return DisplayName; }

	UFUNCTION(BlueprintCallable, Category="Rizz|Info")
	UTexture2D* GetPortrait() const { return Portrait; }

	UFUNCTION(BlueprintCallable, Category="Rizz|Info")
	const FDataTableRowHandle& GetCharacterRow() const { return CharacterRow; }

	// === Initialization ===
	/** Initialize directly from a DataTable row name */
	UFUNCTION(BlueprintCallable, Category="Rizz|Init")
	void InitFromDataTable(UDataTable* DataTable, FName RowName);

	virtual void OnConstruction(const FTransform& Transform) override;
	
	//For walking ab
	bool bIsItNPC = false;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void NotifyControllerChanged() override;

	// === Core Components ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|Turn")
	TObjectPtr<URTurnComponent> TurnComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|Inventory")
	TObjectPtr<URInventoryComponent> InventoryComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|Equipment")
	TObjectPtr<UREquipmentComponent> EquipmentComponent = nullptr;

	// === Death — Animation ===

	/** Death animation. Null = skip montage and call FinishDeath immediately. */
	UPROPERTY(EditDefaultsOnly, Category="Rizz|Death|Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	// === Death — Physics ===

	/** Enable full-body physics ragdoll after FinishDeath. Default OFF — character stays in death pose. */
	UPROPERTY(EditDefaultsOnly, Category="Rizz|Death")
	bool bEnableRagdollOnDeath = false;

	/** Seconds after FinishDeath before ragdoll activates. */
	UPROPERTY(EditDefaultsOnly, Category="Rizz|Death",
		meta=(ClampMin="0.0", EditCondition="bEnableRagdollOnDeath"))
	float RagdollDelay = 0.f;

	// === Death — Actor Lifetime ===

	/** Destroy the actor after DestroyDelay seconds following FinishDeath. */
	UPROPERTY(EditDefaultsOnly, Category="Rizz|Death")
	bool bDestroyOnDeath = false;

	/** Seconds after FinishDeath before Destroy() is called. */
	UPROPERTY(EditDefaultsOnly, Category="Rizz|Death",
		meta=(ClampMin="0.0", EditCondition="bDestroyOnDeath"))
	float DestroyDelay = 3.f;

	// === Death — Loot ===

	/** When true, this corpse can be looted by the player after death. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|Death|Loot")
	bool bLootableOnDeath = false;

	/** Items available on the corpse. Set defaults in the BP class, override per-instance in the level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|Death|Loot",
		meta=(EditCondition="bLootableOnDeath", ShowOnlyInnerProperties))
	FLootSet DeathLoot;

	/** Interaction radius around the corpse in world units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|Death|Loot",
		meta=(ClampMin="50.0", EditCondition="bLootableOnDeath"))
	float CorpseLootRadius = 150.f;

	/** Runtime flag — true once all DeathLoot entries are taken. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Rizz|Death|Loot")
	bool bCorpseLootTaken = false;

	// === Death — Blueprint Events ===

	/** Fired when the player opens (interacts with) the corpse loot. Use to open the loot UI. */
	UFUNCTION(BlueprintImplementableEvent, Category="Rizz|Death|Loot")
	void BP_OnCorpseLootOpened(AActor* InteractingActor);

	/** Fired when all loot has been taken. Use to hide the interact prompt. */
	UFUNCTION(BlueprintImplementableEvent, Category="Rizz|Death|Loot")
	void BP_OnCorpseLootEmptied(AActor* InteractingActor);

	/** Pick a DataTable + row to define this character. Updates visuals in editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rizz|Config", meta=(RowType="/Script/RizzGame.CharacterDefinitionRow"))
	FDataTableRowHandle CharacterRow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rizz|AI")
	TObjectPtr<AAIController> AIController = nullptr;

	// === Cached Attribute Sets ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rizz|Attributes")
	TObjectPtr<URCharacterCoreAttributeSet> CoreAttributeSet = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rizz|Attributes")
	TObjectPtr<URCharacterCombatAttributeSet> CombatAttributeSet = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rizz|Attributes")
	TObjectPtr<URCharacterTurnAttributeSet> TurnAttributeSet = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rizz|Attributes")
	TObjectPtr<URCharacterDebuffAttributeSet> DebuffAttributeSet = nullptr;

	// === Character Info (loaded from DataTable) ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rizz|Info")
	FText DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rizz|Info")
	TObjectPtr<UTexture2D> Portrait = nullptr;

private:
	/** Internal initialization from a definition row */
	void InitFromDefinition(const struct FCharacterDefinitionRow* Def);

	/** Apply only visuals from a definition row (safe for editor/OnConstruction) */
	void ApplyVisualsFromDefinition(const struct FCharacterDefinitionRow* Def);

	bool bIsDead = false;

	UPROPERTY()
	TObjectPtr<USplineComponent> PathSpline;

	UPROPERTY()
	TArray<TObjectPtr<USplineMeshComponent>> PathSegments;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> PathWaypoints;

	TArray<FVector> LastPreviewPoints;
	bool bPathPreviewVisible = false;
	bool bWalkPathFrozen = false;

	bool bDeathFinished = false;
	FTimerHandle DeathTimerHandle;

	void PlayDeathMontage();
	void StartRagdoll();
	void FinishDeath_OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// IRInteractable — active only when bIsDead && bLootableOnDeath
	virtual bool CanInteract_Implementation(APawn* InteractingPawn) override;
	virtual void Interact_Implementation(APawn* InteractingPawn) override;
	virtual FText GetInteractionName_Implementation() override;
	virtual FText GetInteractionAction_Implementation() override;
	virtual float GetInteractionRadius_Implementation() override;
	virtual FVector GetInteractionLocation_Implementation() override;
};
