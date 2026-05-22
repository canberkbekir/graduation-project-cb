// RUILayerController.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/Common/RGameHUD.h"
#include "RUILayerController.generated.h"

struct FCombatStarted;
struct FCombatEnded;

UCLASS()
class RIZZGAME_API URUILayerController : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void RegisterGameHUD(URGameHUD* InHUD, APlayerController* OwningPC);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "R|UI")
	bool IsInCombat() const { return bInCombat; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "R|UI")
	URGameHUD* GetGameHUD() const;

	UFUNCTION(BlueprintCallable, Category = "R|UI")
	void ShowPopup(UUserWidget* PopupWidget);

	UFUNCTION(BlueprintCallable, Category = "R|UI")
	void ClosePopup(UUserWidget* PopupWidget);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "R|UI")
	bool HasActivePopup() const;

private:
	void OnCombatStarted(const FCombatStarted& Event);
	void OnCombatEnded(const FCombatEnded& Event);
	void ApplyDefaultLayerState();
	void UpdateInputMode();

	TWeakObjectPtr<URGameHUD> GameHUD;
	TWeakObjectPtr<APlayerController> CachedPC;

	bool bInCombat = false;
};
