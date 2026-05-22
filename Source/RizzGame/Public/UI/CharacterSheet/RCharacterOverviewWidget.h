// RCharacterOverviewWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RCharacterOverviewWidget.generated.h"

class ARCharacterBase;
class ARCharacterSheetActor;
class URInventoryWidget;
class URCharacterSheetWidget;
class UREquipmentPaperDollWidget;
class URViewNavigationWidget;
class UWidgetSwitcher;
class UTextBlock;

/**
 * Root widget for the character overview screen.
 * Contains character sheet, equipment paper doll, and inventory panels
 * with view navigation and animated transitions.
 */
UCLASS()
class RIZZGAME_API URCharacterOverviewWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /* ───────── Initialization ───────── */

    UFUNCTION(BlueprintCallable, Category = "RizzGame")
    void InitCharacterOverviewWidget(ARCharacterBase* InCharacter, bool bSetInputMode);

    UFUNCTION(BlueprintCallable, Category = "RizzGame")
    void CloseCharacterOverviewWidget();

    /**
     * Set the preview actor for 3D character display.
     */
    UFUNCTION(BlueprintCallable, Category = "RizzGame")
    void SetPreviewActor(ARCharacterSheetActor* InPreviewActor);

    /* ───────── Accessors ───────── */

    UFUNCTION(BlueprintPure, Category = "RizzGame")
    ARCharacterBase* GetCurrentCharacter() const { return CurrentCharacter; }

    UFUNCTION(BlueprintPure, Category = "RizzGame")
    URCharacterSheetWidget* GetCharacterSheetPanel() const { return CharSheetPanel; }

    UFUNCTION(BlueprintPure, Category = "RizzGame")
    URInventoryWidget* GetInventoryPanel() const { return InventoryPanel; }

    UFUNCTION(BlueprintPure, Category = "RizzGame")
    UREquipmentPaperDollWidget* GetCenterPanel() const { return CenterPanel; }

    UFUNCTION(BlueprintPure, Category = "RizzGame")
    URViewNavigationWidget* GetViewNavigation() const { return ViewNavigation; }

    /* ───────── View Navigation ───────── */

    /**
     * Switch to a specific view by ID.
     */
    UFUNCTION(BlueprintCallable, Category = "RizzGame|Navigation")
    void SwitchToView(FName ViewID);

    /**
     * Get the current view ID.
     */
    UFUNCTION(BlueprintPure, Category = "RizzGame|Navigation")
    FName GetCurrentViewID() const;

    /* ───────── Panel Animations ───────── */

    /**
     * Play panel open animations.
     */
    UFUNCTION(BlueprintCallable, Category = "RizzGame|Animation")
    void PlayPanelOpenAnimation();

    /**
     * Play panel close animations.
     */
    UFUNCTION(BlueprintCallable, Category = "RizzGame|Animation")
    void PlayPanelCloseAnimation();

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    /* ───────── Bound Widgets ───────── */

    /** Character name display in header. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "RizzGame|Header")
    TObjectPtr<UTextBlock> Txt_CharacterName = nullptr;

    /** View navigation widget in header. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "RizzGame|Header")
    TObjectPtr<URViewNavigationWidget> ViewNavigation = nullptr;

    /** Left panel - character sheet. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "RizzGame|Panels")
    TObjectPtr<URCharacterSheetWidget> CharSheetPanel;

    /** Center panel - equipment paper doll with 3D preview. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "RizzGame|Panels")
    TObjectPtr<UREquipmentPaperDollWidget> CenterPanel = nullptr;

    /** Right panel - inventory. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "RizzGame|Panels")
    TObjectPtr<URInventoryWidget> InventoryPanel;

    /** Widget switcher for different views (Inventory, Skills, Quests). */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "RizzGame|Panels")
    TObjectPtr<UWidgetSwitcher> ViewSwitcher = nullptr;

    /* ───────── Events ───────── */

    UFUNCTION(BlueprintImplementableEvent, Category = "CharacterMenu|Events")
    void BP_OnOpened(ARCharacterBase* Character);

    UFUNCTION(BlueprintImplementableEvent, Category = "CharacterMenu|Events")
    void BP_OnClosed();

    UFUNCTION(BlueprintImplementableEvent, Category = "CharacterMenu|Events")
    void BP_OnViewChanged(FName NewViewID, const FText& DisplayName);

    /* ───────── Internal ───────── */

    /** Handle view navigation changes. */
    UFUNCTION()
    void HandleViewChanged(FName NewViewID, const FText& DisplayName, int32 ViewIndex);

    /** Initialize all child panels. */
    void InitializePanels();

    /** Update character name display. */
    void UpdateCharacterName();

private:
    UPROPERTY(Transient)
    TObjectPtr<ARCharacterBase> CurrentCharacter = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<ARCharacterSheetActor> PreviewActor = nullptr;
};
