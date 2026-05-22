// RViewNavigationWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RViewNavigationWidget.generated.h"

class UButton;
class UTextBlock;

/** Structure defining a single view entry. */
USTRUCT(BlueprintType)
struct FRViewEntry
{
    GENERATED_BODY()

    /** Unique identifier for this view (e.g., "Inventory", "Skills", "Quests"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View")
    FName ViewID;

    /** Display name shown in the UI. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View")
    FText DisplayName;

    /** Optional icon for this view. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View")
    FSlateBrush Icon;

    FRViewEntry()
        : ViewID(NAME_None)
        , DisplayName(FText::GetEmpty())
    {
    }

    FRViewEntry(FName InID, const FText& InDisplayName)
        : ViewID(InID)
        , DisplayName(InDisplayName)
    {
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnViewChanged, FName, NewViewID, const FText&, DisplayName, int32, ViewIndex);

/**
 * Navigation widget for switching between views.
 * Displays current view title with left/right navigation arrows.
 * Views are defined as FName IDs for maximum flexibility.
 */
UCLASS()
class RIZZGAME_API URViewNavigationWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /* ───────── Configuration ───────── */

    /**
     * List of available views in display order.
     * Add/remove entries in Blueprint to customize available views.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View Navigation")
    TArray<FRViewEntry> Views;

    /** Enable wrapping from last to first view and vice versa. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View Navigation")
    bool bEnableWrapping = true;

    /** Starting view index. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View Navigation", meta = (ClampMin = "0"))
    int32 StartingViewIndex = 0;

    /* ───────── Editor Preview ───────── */

    /** Enable preview in editor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View Navigation|Editor Preview")
    bool bShowEditorPreview = true;

    /* ───────── Navigation ───────── */

    /**
     * Navigate to the previous view.
     */
    UFUNCTION(BlueprintCallable, Category = "View Navigation")
    void NavigateLeft();

    /**
     * Navigate to the next view.
     */
    UFUNCTION(BlueprintCallable, Category = "View Navigation")
    void NavigateRight();

    /**
     * Navigate directly to a view by ID.
     */
    UFUNCTION(BlueprintCallable, Category = "View Navigation")
    void NavigateToView(FName ViewID);

    /**
     * Navigate to a specific view index.
     */
    UFUNCTION(BlueprintCallable, Category = "View Navigation")
    void NavigateToIndex(int32 Index);

    /**
     * Add a new view at runtime.
     */
    UFUNCTION(BlueprintCallable, Category = "View Navigation")
    void AddView(FName ViewID, const FText& DisplayName);

    /**
     * Remove a view by ID.
     */
    UFUNCTION(BlueprintCallable, Category = "View Navigation")
    void RemoveView(FName ViewID);

    /**
     * Get the current view ID.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "View Navigation")
    FName GetCurrentViewID() const;

    /**
     * Get the current view entry.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "View Navigation")
    FRViewEntry GetCurrentView() const;

    /**
     * Get the current view index.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "View Navigation")
    int32 GetCurrentViewIndex() const { return CurrentViewIndex; }

    /**
     * Get the total number of views.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "View Navigation")
    int32 GetViewCount() const { return Views.Num(); }

    /**
     * Check if a view exists.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "View Navigation")
    bool HasView(FName ViewID) const;

    /**
     * Get view index by ID. Returns -1 if not found.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "View Navigation")
    int32 GetViewIndex(FName ViewID) const;

    /* ───────── Events ───────── */

    UPROPERTY(BlueprintAssignable, Category = "View Navigation|Events")
    FOnViewChanged OnViewChanged;

    /* ───────── Bound Widgets ───────── */

    /** Text displaying the current view title. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "View Navigation|Bind")
    UTextBlock* Txt_ViewTitle = nullptr;

    /** Left navigation button. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "View Navigation|Bind")
    UButton* Btn_NavLeft = nullptr;

    /** Right navigation button. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "View Navigation|Bind")
    UButton* Btn_NavRight = nullptr;

protected:
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** Update the view title display. */
    void UpdateViewTitle();

    /** Update navigation button states. */
    void UpdateButtonStates();

    /** Set up default views if none configured. */
    void SetupDefaultViews();

private:
    UFUNCTION()
    void HandleNavLeftClicked();

    UFUNCTION()
    void HandleNavRightClicked();

    UPROPERTY()
    int32 CurrentViewIndex = 0;
};
