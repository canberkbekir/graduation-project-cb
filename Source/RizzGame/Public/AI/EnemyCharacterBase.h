

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "NavModifierComponent.h"
#include "Components/RSelectionComponent.h"
#include "Core/RCharacterBase.h"
#include "HAIPro/Public/HAIBaseComponent.h"
#include "CoverSystem/Public/CoverComponent.h"
#include "EnemyCharacterBase.generated.h"


UCLASS()
class RIZZGAME_API AEnemyCharacterBase : public ARCharacterBase, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly,EditAnywhere)
	TObjectPtr<UHAIBaseComponent> HAIBaseComponent;
	UPROPERTY(BlueprintReadOnly,EditAnywhere)
	TObjectPtr<UCoverComponent> CoverComponent;
	UPROPERTY(BlueprintReadOnly,EditAnywhere)
	TObjectPtr<URSelectionComponent> SelectionComponent;
	UPROPERTY(BlueprintReadOnly,EditAnywhere)
	TObjectPtr<UNavModifierComponent> NavModifierComponent;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer EnemyTags;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UObject> ScramblerAbility;
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class UGameplayAbility> LastPlayedAbilityClass;
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	
private:
	AEnemyCharacterBase();
	
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnTurnStarted();
	UFUNCTION()
	void OnTurnEnded();
	UFUNCTION()
	void OutFromCover(ECoverType CoverType, uint8 NewCoverPosition);
	UFUNCTION()
	void OnCoverOccupied(ACoverPoint* NewCoverPoint);
	UFUNCTION()
	void OnCoverFreed(ACoverPoint* FreedCoverPoint);
	UFUNCTION()
	void OnPerceptionConfigUpdated(const F_SenseData& SenseData);
	UFUNCTION()
	void OnSenseActorForgotten(const AActor* ForgottenActor);
};
