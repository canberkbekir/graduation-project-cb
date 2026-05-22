// RDoor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interaction/RInteractable.h"
#include "RDoor.generated.h"

class URItemDefinition;
class USceneComponent;
class UStaticMeshComponent;
class USphereComponent;
class ARFogRoom;

UCLASS()
class RIZZGAME_API ARDoor : public AActor, public IRInteractable
{
	GENERATED_BODY()

public:
	ARDoor();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Rizz|Door")
	bool TryOpen(AActor* InteractingActor);

	UFUNCTION(BlueprintCallable, Category = "Rizz|Door")
	void Close();

	bool IsOpen() const { return bIsOpen; }
	bool IsLocked() const { return bIsLocked; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rizz|Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rizz|Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rizz|Components")
	TObjectPtr<USphereComponent> InteractionArea;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|Door",
	          meta = (Categories = "FogRoom", ToolTip = "Unique tag for this door. Sent with FRDoorOpened/FRDoorClosed events."))
	FGameplayTag DoorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rizz|Door")
	TArray<TObjectPtr<ARFogRoom>> ConnectedRooms;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|Door")
	bool bStartsOpen = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Rizz|Door")
	bool bIsOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|Door")
	bool bIsLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|Door",
	          meta = (EditCondition = "bIsLocked"))
	TObjectPtr<URItemDefinition> RequiredKeyItem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rizz|Door",
	          meta = (ToolTip = "When closed, this door's mesh blocks LOS raycasts. Opening it makes traces pass through."))
	bool bBlocksLOSWhenClosed = true;

	UFUNCTION(BlueprintImplementableEvent, Category = "Rizz|Events")
	void BP_OnDoorOpened(AActor* InteractingActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Rizz|Events")
	void BP_OnDoorClosed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Rizz|Events")
	void BP_OnDoorOpenFailedLocked(AActor* InteractingActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Rizz|Events")
	bool BP_TryUseRequiredKey(AActor* InteractingActor);

	virtual bool CanInteract_Implementation(APawn* InteractingPawn) override;
	virtual void Interact_Implementation(APawn* InteractingPawn) override;
	virtual FText GetInteractionName_Implementation() override;
	virtual FText GetInteractionAction_Implementation() override;
	virtual float GetInteractionRadius_Implementation() override;
	virtual FVector GetInteractionLocation_Implementation() override;

private:
	void SetLOSCollision(bool bBlock);
};
