// RButtonGridWidget.h
// -----------------------------------------------------------------------------
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RButtonGridWidget.generated.h"

class UUniformGridPanel;
class UUniformGridSlot;
class USizeBox;
class UBorder;

/**
 * Defines how the grid is filled with children.
 */
UENUM(BlueprintType)
enum class EFillDirection : uint8
{
	/** Fill left→right across a row, then move to the next row (like reading text). */
	RowMajor UMETA(DisplayName="Row Major"),

	/** Fill top→bottom down a column, then move to the next column. */
	ColumnMajor UMETA(DisplayName="Column Major")
};

/**
 * A flexible button grid widget based on a UniformGridPanel.
 *
 * Features:
 * - Control number of rows and columns
 * - Auto-compute rows if set to 0
 * - Choose whether to fill by row or by column
 * - Custom slot padding and alignment
 * - Fixed cell height + auto width via SizeBox wrapper
 * - Simple API to add/remove/reflow items
 */
UCLASS()
class RIZZGAME_API URButtonGridWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** The underlying grid panel (must be bound in the widget Blueprint). */
	UPROPERTY(meta=(BindWidget))
	UUniformGridPanel* UG_Grid = nullptr;

	/** Number of columns (>=1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	int32 Columns = 3;

	/** Number of rows (0 = auto). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	int32 Rows = 0;

	/** Fill direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	EFillDirection FillDirection = EFillDirection::RowMajor;

	/** Padding applied around each child (via wrapper Border). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	FMargin SlotPadding = FMargin(4.f);

	/** Horizontal alignment inside each cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	TEnumAsByte<EHorizontalAlignment> HAlign = HAlign_Center;

	/** Vertical alignment inside each cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	TEnumAsByte<EVerticalAlignment> VAlign = VAlign_Center;

	/** Fixed height for each cell (applied via SizeBox wrapper). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	float FixedCellHeight = 64.f;

	/** Also set grid's MinDesiredSlotHeight to FixedCellHeight for extra stability. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	bool bApplyGridMinSlotHeight = true;

	/** Rearranges current children according to Columns/Rows/FillDirection. */
	UFUNCTION(BlueprintCallable, Category="Grid")
	void ReflowChildren();

	/** Clears existing children, adds the given array, and reflows. */
	UFUNCTION(BlueprintCallable, Category="Grid")
	void SetItems(const TArray<UWidget*>& Widgets);

	/** Adds a single widget (wrapped) and reflows. */
	UFUNCTION(BlueprintCallable, Category="Grid")
	void AddItem(UWidget* Widget);

	/** Removes all current children (panel remains). */
	UFUNCTION(BlueprintCallable, Category="Grid")
	void ClearItems();

	/** Update column count and reflow. */
	UFUNCTION(BlueprintCallable, Category="Grid")
	void SetColumnsAndReflow(int32 NewColumns);

	/** Update row count and reflow. */
	UFUNCTION(BlueprintCallable, Category="Grid")
	void SetRowsAndReflow(int32 NewRows);

protected:
	/** Editor preview refresh. */
	virtual void NativePreConstruct() override;

	/** Runtime init. */
	virtual void NativeConstruct() override;

private:
	/** Wrap a child with SizeBox (height override) and Border (padding & alignment). */
	UWidget* MakeWrappedForGrid(UWidget* Child) const;

	/** Apply grid-level hints (optional). */
	void ApplyGridHints() const;
};
