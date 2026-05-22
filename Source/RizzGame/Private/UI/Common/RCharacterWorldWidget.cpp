// RCharacterWorldWidget.cpp

#include "UI/Common/RCharacterWorldWidget.h"
#include "Core/RCharacterBase.h"
#include "GAS/RStatusEffectLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/WidgetSwitcher.h"

void URCharacterWorldWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Collapsed);
	OnCloseAnimationComplete.AddDynamic(this, &URCharacterWorldWidget::HandleCloseComplete);
}

void URCharacterWorldWidget::HandleCloseComplete()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void URCharacterWorldWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}
	Super::NativeDestruct();
}

void URCharacterWorldWidget::InitForCharacter(ARCharacterBase* InCharacter)
{
	OwnerCharacter = InCharacter;
	BP_OnInitialized(InCharacter);
}

void URCharacterWorldWidget::OnCursorEnter()
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RefreshTimerHandle,
			this,
			&URCharacterWorldWidget::TickEffectRefresh,
			EffectRefreshInterval,
			true,
			0.f);
	}

	PlayOpenAnimation();
	BP_OnHoverBegin();
}

void URCharacterWorldWidget::OnCursorLeave()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}

	// If the open animation hadn't finished yet, skip the close animation entirely.
	if (IsAnimating() && !IsOpen())
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		PlayCloseAnimation();
	}

	BP_OnHoverEnd();
}

TArray<FEffectIconViewModel> URCharacterWorldWidget::GetActiveEffects() const
{
	if (!OwnerCharacter || !StatusEffectDisplayTable)
	{
		return {};
	}

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		return {};
	}

	return URStatusEffectLibrary::GetActiveEffectIcons(ASC, StatusEffectDisplayTable);
}

void URCharacterWorldWidget::TickEffectRefresh()
{
	BP_OnEffectsRefreshed(GetActiveEffects());
}

// ── Page navigation ───────────────────────────────────────────────────────────

void URCharacterWorldWidget::GoToPage(int32 PageIndex)
{
	if (!WS_Pages)
	{
		return;
	}

	const int32 Total = WS_Pages->GetChildrenCount();
	if (Total == 0)
	{
		return;
	}

	const int32 Clamped = FMath::Clamp(PageIndex, 0, Total - 1);
	WS_Pages->SetActiveWidgetIndex(Clamped);
	BP_OnPageChanged(Clamped, Total);
}

void URCharacterWorldWidget::NextPage()
{
	if (!WS_Pages)
	{
		return;
	}

	const int32 Total = WS_Pages->GetChildrenCount();
	if (Total == 0)
	{
		return;
	}

	const int32 Next = (WS_Pages->GetActiveWidgetIndex() + 1) % Total;
	GoToPage(Next);
}

void URCharacterWorldWidget::PrevPage()
{
	if (!WS_Pages)
	{
		return;
	}

	const int32 Total = WS_Pages->GetChildrenCount();
	if (Total == 0)
	{
		return;
	}

	const int32 Prev = (WS_Pages->GetActiveWidgetIndex() - 1 + Total) % Total;
	GoToPage(Prev);
}

int32 URCharacterWorldWidget::GetCurrentPage() const
{
	return WS_Pages ? WS_Pages->GetActiveWidgetIndex() : 0;
}

int32 URCharacterWorldWidget::GetPageCount() const
{
	return WS_Pages ? WS_Pages->GetChildrenCount() : 0;
}
