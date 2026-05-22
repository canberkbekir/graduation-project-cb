#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/LootType.h"
#include "Interaction/RInteractable.h"
#include "RLootContainer.generated.h"

class URItemDefinition;
class USceneComponent;
class UStaticMeshComponent;
class URInteractionAreaComponent;

UCLASS()
class RIZZGAME_API ARLootContainer : public AActor, public IRInteractable
{
	GENERATED_BODY()

public:
	ARLootContainer();

protected:
	/* ---------- Components ---------- */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rizz|Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	// Visual mesh for the container.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rizz|Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ContainerMesh;

	// Defines the region from which this container can be interacted with.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rizz|Components", meta = (AllowPrivateAccess = "true"))
	URInteractionAreaComponent* InteractionArea;

	/* ---------- Loot Data ---------- */

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|Loot", meta = (ShowOnlyInnerProperties))
	FLootSet Loot;

	/* ---------- State ---------- */

	// Treat this container as already opened when the level starts.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|State")
	bool bStartsOpened = false;

	// Runtime flag — true after the first successful open.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Rizz|State")
	bool bHasBeenOpened = false;

	// Prevents opening without the required key or a successful alternative unlock.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|State")
	bool bIsLocked = false;

	// Item that must be in the opener's inventory to unlock this container.
	// Leave empty to rely on BP_TryAlternativeUnlock instead.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|State", meta = (EditCondition = "bIsLocked"))
	URItemDefinition* RequiredKeyItem = nullptr;

	// Remove one of the required key item from the opener's inventory after a successful unlock.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|State", meta = (EditCondition = "bIsLocked && RequiredKeyItem != nullptr", EditConditionHides))
	bool bConsumeKeyOnUse = true;

	// Automatically move all loot into the opener's inventory the first time this container is opened.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|State")
	bool bConsumeLootOnFirstOpen = false;

	/* ---------- Debug ---------- */

	// Draw the interaction radius as a debug sphere (non-shipping builds only).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|Debug")
	bool bShowInteractionArea = false;

public:
	/* ---------- AActor ---------- */

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/* ---------- Query API ---------- */

	UFUNCTION(BlueprintCallable, Category = "Rizz|Loot")
	const FLootSet& GetLootSet() const { return Loot; }

	UFUNCTION(BlueprintCallable, Category = "Rizz|Loot")
	void GetLootEntries(TArray<FLootEntry>& OutEntries) const;

	UFUNCTION(BlueprintCallable, Category = "Rizz|Loot")
	bool IsEmpty() const;

	UFUNCTION(BlueprintCallable, Category = "Rizz|Loot")
	void ClearLoot(AActor* InteractingActor);

	UFUNCTION(BlueprintCallable, Category = "Rizz|State")
	bool IsLocked() const { return bIsLocked; }

	UFUNCTION(BlueprintCallable, Category = "Rizz|State")
	bool HasBeenOpened() const { return bHasBeenOpened; }

	UFUNCTION(BlueprintCallable, Category = "Rizz|State")
	void SetLocked(bool bNewLocked);

	/* ---------- Interaction API ---------- */

	/** Handles lock checks, first-open events, and optional auto-loot. */
	UFUNCTION(BlueprintCallable, Category = "Rizz|Interaction")
	bool TryOpen(AActor* InteractingActor);

	/**
	 * Moves all loot into the InteractingActor's inventory.
	 * Entries that don't fit (capacity full) are left behind.
	 * Returns true if at least one item was transferred.
	 * Fires BP_OnLootEmptied if the container becomes empty.
	 */
	UFUNCTION(BlueprintCallable, Category = "Rizz|Loot")
	bool TakeAllLoot(AActor* InteractingActor);

protected:
	/* ---------- Blueprint Events ---------- */

	// Called once on the very first successful open.
	UFUNCTION(BlueprintImplementableEvent, Category = "Rizz|Events")
	void BP_OnOpenedFirstTime(AActor* InteractingActor);

	// Called every time the container is successfully opened.
	UFUNCTION(BlueprintImplementableEvent, Category = "Rizz|Events")
	void BP_OnOpened(AActor* InteractingActor);

	// Called when opening fails because the container is locked.
	UFUNCTION(BlueprintImplementableEvent, Category = "Rizz|Events")
	void BP_OnOpenFailedLocked(AActor* InteractingActor);

	// Called when the container's loot becomes empty.
	UFUNCTION(BlueprintImplementableEvent, Category = "Rizz|Events")
	void BP_OnLootEmptied(AActor* InteractingActor);

	/**
	 * Checks the opener's inventory for RequiredKeyItem and consumes one if found.
	 * Override in Blueprint to add custom key logic (faction checks, unbreakable keys, etc.).
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Rizz|Events")
	bool BP_TryUseRequiredKey(AActor* InteractingActor);

	/**
	 * Called when the container is locked but has no RequiredKeyItem set.
	 * Override in Blueprint to implement lockpicking, force-open, magic unlock, etc.
	 * Return true if the container was successfully unlocked; default returns false.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Rizz|Events")
	bool BP_TryAlternativeUnlock(AActor* InteractingActor);

	/* ---------- IRInteractable ---------- */

	virtual bool CanInteract_Implementation(APawn* InteractingPawn) override;
	virtual void Interact_Implementation(APawn* InteractingPawn) override;
	virtual FText GetInteractionName_Implementation() override;
	virtual FText GetInteractionAction_Implementation() override;
	virtual float GetInteractionRadius_Implementation() override;
	virtual FVector GetInteractionLocation_Implementation() override;
};
