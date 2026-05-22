// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/StateTreeComponent.h"
#include "RTurnComponent.generated.h"

UENUM(BlueprintType)
enum class ERCombatTeam : uint8
{
	Player,
	Enemy,
	Neutral
};

UENUM(BlueprintType)
enum class ETurnState : uint8
{
	OutTurn UMETA(DisplayName = "Out of Turn"),
	InTurn UMETA(DisplayName = "In Turn"),
	Waiting UMETA(DisplayName = "Waiting")
};

USTRUCT(BlueprintType)
struct FTurnData
{
	GENERATED_BODY()

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn")
	int32 ActionPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn")
	float WalkDistanceLeft = 900.f;*/
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTurnStartedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTurnEndedDelegate);


UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RIZZGAME_API URTurnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URTurnComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void StartTurn();
	UFUNCTION(BlueprintCallable)
	void EndTurn();
	void EnterCombat();

	void ExitCombat();

	/*void CacheTurnData();
	void ResetTurnData();*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn")
	ETurnState TurnState = ETurnState::OutTurn;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn")
	FTurnData TurnData;

	FTurnData CachedTurnData;*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn")
	ERCombatTeam Team = ERCombatTeam::Enemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn")
	int32 Initiative = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStateTreeComponent> StateTreeComponent;

	UPROPERTY(BlueprintAssignable, Category = "Turn|Events")
	FTurnStartedDelegate OnTurnStarted;
	UPROPERTY(BlueprintAssignable, Category = "Turn|Events")
	FTurnEndedDelegate OnTurnEnded;
};
