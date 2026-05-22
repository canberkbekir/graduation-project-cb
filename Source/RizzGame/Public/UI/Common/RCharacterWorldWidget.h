// RCharacterWorldWidget.h
#pragma once

#include "CoreMinimal.h"
#include "UI/Common/RAnimatedPanelWidget.h"
#include "UI/Models/RCombatUIModels.h"
#include "RCharacterWorldWidget.generated.h"

class ARCharacterBase;
class UWidgetSwitcher;
class UDataTable;

/**
 * World-space floating info panel attached to ARCharacterBase.
 * Inherits slide/fade/scale animations from URAnimatedPanelWidget.
 *
 * C++ provides: hover entry/exit hooks, periodic effect data push, page navigation.
 * All visual content is built by the user in Blueprint.
 *
 * Quick BP wiring:
 *   BP_OnHoverBegin  -> PlayOpenAnimation
 *   BP_OnHoverEnd    -> PlayCloseAnimation
 *   OnCloseAnimationComplete -> SetVisibility(Collapsed)
 */
UCLASS()
class RIZZGAME_API URCharacterWorldWidget : public URAnimatedPanelWidget
{
	GENERATED_BODY()

public:
	/** Called by ARCharacterBase::BeginPlay after the WidgetComponent creates this widget. */
	UFUNCTION(BlueprintCallable, Category="Rizz|UI")
	void InitForCharacter(ARCharacterBase* InCharacter);

	// ── Page navigation ──────────────────────────────────────────────────────
	/** Switch to a specific page index (0-based). No-op if WS_Pages is not bound. */
	UFUNCTION(BlueprintCallable, Category="Rizz|UI")
	void GoToPage(int32 PageIndex);

	UFUNCTION(BlueprintCallable, Category="Rizz|UI")
	void NextPage();

	UFUNCTION(BlueprintCallable, Category="Rizz|UI")
	void PrevPage();

	UFUNCTION(BlueprintPure, Category="Rizz|UI")
	int32 GetCurrentPage() const;

	UFUNCTION(BlueprintPure, Category="Rizz|UI")
	int32 GetPageCount() const;

	// ── Hover entry/exit (called by owning character) ────────────────────────
	UFUNCTION(BlueprintCallable, Category="Rizz|UI")
	void OnCursorEnter();

	UFUNCTION(BlueprintCallable, Category="Rizz|UI")
	void OnCursorLeave();

	/** Manually query current effects from the owning character's ASC. */
	UFUNCTION(BlueprintCallable, Category="Rizz|UI")
	TArray<FEffectIconViewModel> GetActiveEffects() const;

	// ── BP events ─────────────────────────────────────────────────────────────
	/** Wire this to PlayOpenAnimation() in your BP. */
	UFUNCTION(BlueprintImplementableEvent, Category="Rizz|UI")
	void BP_OnHoverBegin();

	/** Wire this to PlayCloseAnimation() in your BP. */
	UFUNCTION(BlueprintImplementableEvent, Category="Rizz|UI")
	void BP_OnHoverEnd();

	/** Fired once when the widget is bound to its owning character. Do first-time setup here. */
	UFUNCTION(BlueprintImplementableEvent, Category="Rizz|UI")
	void BP_OnInitialized(ARCharacterBase* Character);

	/** Fired every EffectRefreshInterval while hovered — build your effect display here. */
	UFUNCTION(BlueprintImplementableEvent, Category="Rizz|UI")
	void BP_OnEffectsRefreshed(const TArray<FEffectIconViewModel>& Effects);

	/** Fired after GoToPage / NextPage / PrevPage. */
	UFUNCTION(BlueprintImplementableEvent, Category="Rizz|UI")
	void BP_OnPageChanged(int32 NewPageIndex, int32 TotalPages);

	// ── Properties ────────────────────────────────────────────────────────────
	/**
	 * DataTable with FStatusEffectDisplayRow rows.
	 * Required for BP_OnEffectsRefreshed to carry icon/color data.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|UI")
	TObjectPtr<UDataTable> StatusEffectDisplayTable;

	/** How often (in seconds) BP_OnEffectsRefreshed fires while the widget is hovered. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rizz|UI", meta=(ClampMin="0.1"))
	float EffectRefreshInterval = 0.5f;

	/** The character this widget belongs to. Set by InitForCharacter. */
	UPROPERTY(BlueprintReadOnly, Category="Rizz|UI")
	TObjectPtr<ARCharacterBase> OwnerCharacter;

protected:
	/**
	 * Optional — add a WidgetSwitcher named exactly "WS_Pages" in your BP
	 * to enable page navigation via GoToPage / NextPage / PrevPage.
	 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> WS_Pages;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void TickEffectRefresh();

	UFUNCTION()
	void HandleCloseComplete();

	FTimerHandle RefreshTimerHandle;
};
