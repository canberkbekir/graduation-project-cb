// RButtonGridWidget.cpp
// ----------------------------------------------------------------------------- 

#include "UI/Common/RButtonGridWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"

void URButtonGridWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	ApplyGridHints();
	ReflowChildren(); // live update in designer
}

void URButtonGridWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ApplyGridHints();
	ReflowChildren();
}

void URButtonGridWidget::SetColumnsAndReflow(int32 NewColumns)
{
	Columns = FMath::Max(1, NewColumns);
	ReflowChildren();
}

void URButtonGridWidget::SetRowsAndReflow(int32 NewRows)
{
	Rows = FMath::Max(0, NewRows); // 0 = auto
	ReflowChildren();
}

void URButtonGridWidget::ClearItems()
{
	if (!UG_Grid)
	{
		return;
	}
	UG_Grid->ClearChildren();
}

void URButtonGridWidget::SetItems(const TArray<UWidget*>& Widgets)
{
	if (!UG_Grid)
	{
		return;
	}
	UG_Grid->ClearChildren();

	for (UWidget* W : Widgets)
	{
		if (!W)
		{
			continue;
		}
		if (UWidget* Wrapped = MakeWrappedForGrid(W))
		{
			UG_Grid->AddChild(Wrapped);
		}
	}

	ReflowChildren();
}

void URButtonGridWidget::AddItem(UWidget* Widget)
{
	if (!UG_Grid || !Widget)
	{
		return;
	}

	if (UWidget* Wrapped = MakeWrappedForGrid(Widget))
	{
		UG_Grid->AddChild(Wrapped);
		ReflowChildren();
	}
}

UWidget* URButtonGridWidget::MakeWrappedForGrid(UWidget* Child) const
{
	if (!Child || !WidgetTree)
	{
		return nullptr;
	}

	// Border for padding + alignment
	UBorder* PaddingWrap = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	PaddingWrap->SetPadding(SlotPadding);
	PaddingWrap->SetHorizontalAlignment(HAlign);
	PaddingWrap->SetVerticalAlignment(VAlign);
	PaddingWrap->SetContent(Child);

	// SizeBox to enforce fixed height, leave width auto
	USizeBox* SizeWrap = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SizeWrap->SetHeightOverride(FixedCellHeight);
	SizeWrap->ClearWidthOverride(); // ensure no fixed width is imposed
	SizeWrap->AddChild(PaddingWrap);

	return SizeWrap;
}

void URButtonGridWidget::ApplyGridHints() const
{
	if (!UG_Grid)
	{
		return;
	}

	if (bApplyGridMinSlotHeight)
	{
		// Helps Slate choose stable uniform cell height at runtime
		UG_Grid->SetMinDesiredSlotHeight(FixedCellHeight);
	}
	// Leave MinDesiredSlotWidth at 0 to keep width flexible (auto)
}

void URButtonGridWidget::ReflowChildren()
{
	if (!UG_Grid)
	{
		return;
	}

	const int32 ChildCount = UG_Grid->GetChildrenCount();
	const int32 TotalCols = FMath::Max(1, Columns);
	const int32 TotalRows = (Rows > 0)
		                        ? Rows
		                        : FMath::CeilToInt(static_cast<float>(ChildCount) / static_cast<float>(TotalCols));

	for (int32 i = 0; i < ChildCount; ++i)
	{
		UWidget* Child = UG_Grid->GetChildAt(i);
		if (!Child)
		{
			continue;
		}

		int32 Row = 0, Col = 0;
		if (FillDirection == EFillDirection::RowMajor)
		{
			Row = i / TotalCols;
			Col = i % TotalCols;
		}
		else
		{
			const int32 SafeRows = FMath::Max(1, TotalRows);
			Col = i / SafeRows;
			Row = i % SafeRows;
		}

		if (UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(Child->Slot))
		{
			GridSlot->SetRow(Row);
			GridSlot->SetColumn(Col);
			GridSlot->SetHorizontalAlignment(HAlign);
			GridSlot->SetVerticalAlignment(VAlign);
			// Padding is handled by the inner Border wrapper.
		}
	}

	// Force a fresh layout pass so PIE matches Designer
	UG_Grid->InvalidateLayoutAndVolatility();
	InvalidateLayoutAndVolatility();
}
