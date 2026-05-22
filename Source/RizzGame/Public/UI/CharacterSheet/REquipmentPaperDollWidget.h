// REquipmentPaperDollWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/RItemType.h"
#include "REquipmentPaperDollWidget.generated.h"

class ARCharacterBase;
class ARCharacterSheetActor;
class UREquipmentComponent;
class UREquipmentSlotWidget;
class UImage;
class UTextureRenderTarget2D;

/**
 * Widget displaying the character 3D preview with equipment slots arranged around it.
 * Supports mouse drag rotation of the 3D preview.
 */
UCLASS()
class RIZZGAME_API UREquipmentPaperDollWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /* ───────── Editor Preview ───────── */

    /** Enable preview in editor for all equipment slots. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paper Doll|Editor Preview")
    bool bShowEditorPreview = true;

    /** Number of slots to show as "filled" in preview (0-6). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paper Doll|Editor Preview", meta = (ClampMin = "0", ClampMax = "6", EditCondition = "bShowEditorPreview"))
    int32 EditorPreviewFilledSlots = 3;

    /* ───────── Initialization ───────── */

    /**
     * Initialize the paper doll with a character and preview actor.
     */
    UFUNCTION(BlueprintCallable, Category = "Paper Doll")
    void Init(ARCharacterBase* InCharacter, ARCharacterSheetActor* InPreviewActor);

    /**
     * Refresh the display from current character state.
     */
    UFUNCTION(BlueprintCallable, Category = "Paper Doll")
    void Refresh();

    /**
     * Get the bound character.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Paper Doll")
    ARCharacterBase* GetCharacter() const { return Character; }

    /**
     * Get the preview actor.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Paper Doll")
    ARCharacterSheetActor* GetPreviewActor() const { return PreviewActor; }

    /* ───────── 3D Preview Control ───────── */

    /** Sensitivity for mouse drag rotation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paper Doll|Preview")
    float RotationSensitivity = 0.5f;

    /** Enable/disable mouse drag rotation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paper Doll|Preview")
    bool bEnableDragRotation = true;

    /**
     * Set the preview rotation angle.
     */
    UFUNCTION(BlueprintCallable, Category = "Paper Doll|Preview")
    void SetPreviewRotation(float YawDegrees);

    /**
     * Add rotation to the preview.
     */
    UFUNCTION(BlueprintCallable, Category = "Paper Doll|Preview")
    void AddPreviewRotation(float DeltaYaw);

    /**
     * Reset preview to default rotation.
     */
    UFUNCTION(BlueprintCallable, Category = "Paper Doll|Preview")
    void ResetPreviewRotation();

    /* ───────── Equipment Slots ───────── */

    /**
     * Get a specific equipment slot widget.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Paper Doll")
    UREquipmentSlotWidget* GetEquipmentSlot(EREquipmentSlot SlotType) const;

    /* ───────── Bound Widgets ───────── */

    /** Image displaying the 3D character preview render target. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Paper Doll|Bind")
    UImage* Img_CharacterPreview = nullptr;

    /** Equipment slot widgets for each slot type. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Paper Doll|Bind")
    UREquipmentSlotWidget* Slot_Head = nullptr;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Paper Doll|Bind")
    UREquipmentSlotWidget* Slot_Chest = nullptr;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Paper Doll|Bind")
    UREquipmentSlotWidget* Slot_Hands = nullptr;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Paper Doll|Bind")
    UREquipmentSlotWidget* Slot_Feet = nullptr;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Paper Doll|Bind")
    UREquipmentSlotWidget* Slot_MainHand = nullptr;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Paper Doll|Bind")
    UREquipmentSlotWidget* Slot_OffHand = nullptr;

protected:
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** Set up editor preview on slots. */
    void CreateEditorPreview();

    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    /** Bind equipment slots to the equipment component. */
    void BindEquipmentSlots();

    /** Unbind equipment slots. */
    void UnbindEquipmentSlots();

    /** Set up the render target on the preview image. */
    void SetupRenderTarget();

private:
    UPROPERTY()
    TObjectPtr<ARCharacterBase> Character = nullptr;

    UPROPERTY()
    TObjectPtr<ARCharacterSheetActor> PreviewActor = nullptr;

    UPROPERTY()
    TObjectPtr<UREquipmentComponent> EquipmentComponent = nullptr;

    /** Whether we're currently dragging to rotate. */
    bool bIsDragging = false;

    /** Last mouse position for drag calculation. */
    FVector2D LastMousePosition;

    /** Default preview rotation. */
    float DefaultRotation = 0.0f;
};
