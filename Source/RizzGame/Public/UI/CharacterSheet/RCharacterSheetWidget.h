// RCharacterSheetWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"
#include "RCharacterSheetWidget.generated.h"

class ARCharacterBase;
class URCharacterPortraitWidget;
class URCharacterSheetAbilitiesWidget;
class URCharacterSheetResistanceWidget;
class URCharacterSheetTabWidget;
struct FCharacterDefinitionRow;

/**
 * Left panel widget displaying character stats and info.
 * Contains portrait, abilities, resistances, and tabs for skills/weapons.
 */
UCLASS()
class RIZZGAME_API URCharacterSheetWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /* ───────── Initialization ───────── */

    /**
     * Initialize the character sheet from a character.
     */
    UFUNCTION(BlueprintCallable, Category = "Character Sheet")
    void InitFromCharacter(ARCharacterBase* InCharacter);

    /**
     * Initialize from a definition row.
     */
    UFUNCTION(BlueprintCallable, Category = "Character Sheet")
    void InitFromDefinition(const FCharacterDefinitionRow& InDef);

    /**
     * Refresh all displayed data.
     */
    UFUNCTION(BlueprintCallable, Category = "Character Sheet")
    void Refresh();

    /**
     * Clear all displayed data.
     */
    UFUNCTION(BlueprintCallable, Category = "Character Sheet")
    void Clear();

    /**
     * Get the current character.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character Sheet")
    ARCharacterBase* GetCharacter() const { return Character; }

    /* ───────── Animation Hooks ───────── */

    /**
     * Play the slide-in animation.
     */
    UFUNCTION(BlueprintCallable, Category = "Character Sheet|Animation")
    void PlaySlideInAnimation();

    /**
     * Play the slide-out animation.
     */
    UFUNCTION(BlueprintCallable, Category = "Character Sheet|Animation")
    void PlaySlideOutAnimation();

    /* ───────── Bound Widgets ───────── */

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Character Sheet|Bind")
    URCharacterPortraitWidget* PortraitWidget = nullptr;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Character Sheet|Bind")
    URCharacterSheetAbilitiesWidget* AbilitiesWidget = nullptr;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Character Sheet|Bind")
    URCharacterSheetResistanceWidget* ResistanceWidget = nullptr;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Character Sheet|Bind")
    URCharacterSheetTabWidget* TabWidget = nullptr;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** Blueprint hook for slide-in animation. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Character Sheet|Animation")
    void BP_PlaySlideInAnimation();

    /** Blueprint hook for slide-out animation. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Character Sheet|Animation")
    void BP_PlaySlideOutAnimation();

    /** Update portrait widget from character. */
    void UpdatePortrait();

    /** Update abilities widget from character. */
    void UpdateAbilities();

    /** Update resistances widget from character. */
    void UpdateResistances();

private:
    UPROPERTY()
    TObjectPtr<ARCharacterBase> Character = nullptr;
};
