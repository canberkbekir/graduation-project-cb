#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Components/RTurnComponent.h"
#include "RCombatUIModels.generated.h"

USTRUCT(BlueprintType)
struct FEffectIconViewModel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Stacks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TintColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TurnsLeft = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInfinite = false;
};

USTRUCT(BlueprintType)
struct FPortraitViewModel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* PortraitTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentHealth = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxHealth = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEffectIconViewModel> StatusEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentKineticShields = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxKineticShields = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentEnergyShields = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxEnergyShields = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERCombatTeam Team = ERCombatTeam::Enemy;

};
