// RViewNavigationWidget.cpp

#include "UI/CharacterSheet/RViewNavigationWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void URViewNavigationWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    // Set up defaults if empty
    if (Views.Num() == 0)
    {
        SetupDefaultViews();
    }

    // Show editor preview
    if (IsDesignTime() && bShowEditorPreview)
    {
        CurrentViewIndex = FMath::Clamp(StartingViewIndex, 0, FMath::Max(0, Views.Num() - 1));
        UpdateViewTitle();
    }
}

void URViewNavigationWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Set up defaults if empty
    if (Views.Num() == 0)
    {
        SetupDefaultViews();
    }

    // Initialize to starting view
    CurrentViewIndex = FMath::Clamp(StartingViewIndex, 0, FMath::Max(0, Views.Num() - 1));

    // Bind button events
    if (Btn_NavLeft)
    {
        Btn_NavLeft->OnClicked.AddDynamic(this, &URViewNavigationWidget::HandleNavLeftClicked);
    }

    if (Btn_NavRight)
    {
        Btn_NavRight->OnClicked.AddDynamic(this, &URViewNavigationWidget::HandleNavRightClicked);
    }

    // Initialize display
    UpdateViewTitle();
    UpdateButtonStates();
}

void URViewNavigationWidget::NativeDestruct()
{
    // Unbind button events
    if (Btn_NavLeft)
    {
        Btn_NavLeft->OnClicked.RemoveDynamic(this, &URViewNavigationWidget::HandleNavLeftClicked);
    }

    if (Btn_NavRight)
    {
        Btn_NavRight->OnClicked.RemoveDynamic(this, &URViewNavigationWidget::HandleNavRightClicked);
    }

    Super::NativeDestruct();
}

void URViewNavigationWidget::SetupDefaultViews()
{
    Views.Add(FRViewEntry(FName("Inventory"), FText::FromString(TEXT("Inventory"))));
    Views.Add(FRViewEntry(FName("Skills"), FText::FromString(TEXT("Skills"))));
    Views.Add(FRViewEntry(FName("Quests"), FText::FromString(TEXT("Quests"))));
}

void URViewNavigationWidget::NavigateLeft()
{
    if (Views.Num() == 0)
    {
        return;
    }

    int32 NewIndex = CurrentViewIndex - 1;

    if (NewIndex < 0)
    {
        if (bEnableWrapping)
        {
            NewIndex = Views.Num() - 1;
        }
        else
        {
            return;
        }
    }

    NavigateToIndex(NewIndex);
}

void URViewNavigationWidget::NavigateRight()
{
    if (Views.Num() == 0)
    {
        return;
    }

    int32 NewIndex = CurrentViewIndex + 1;

    if (NewIndex >= Views.Num())
    {
        if (bEnableWrapping)
        {
            NewIndex = 0;
        }
        else
        {
            return;
        }
    }

    NavigateToIndex(NewIndex);
}

void URViewNavigationWidget::NavigateToView(FName ViewID)
{
    int32 Index = GetViewIndex(ViewID);
    if (Index != INDEX_NONE)
    {
        NavigateToIndex(Index);
    }
}

void URViewNavigationWidget::NavigateToIndex(int32 Index)
{
    if (Index < 0 || Index >= Views.Num())
    {
        return;
    }

    if (Index == CurrentViewIndex)
    {
        return;
    }

    CurrentViewIndex = Index;

    UpdateViewTitle();
    UpdateButtonStates();

    // Broadcast change
    const FRViewEntry& CurrentView = Views[CurrentViewIndex];
    OnViewChanged.Broadcast(CurrentView.ViewID, CurrentView.DisplayName, CurrentViewIndex);
}

void URViewNavigationWidget::AddView(FName ViewID, const FText& DisplayName)
{
    // Check if already exists
    if (HasView(ViewID))
    {
        return;
    }

    Views.Add(FRViewEntry(ViewID, DisplayName));
    UpdateButtonStates();
}

void URViewNavigationWidget::RemoveView(FName ViewID)
{
    int32 Index = GetViewIndex(ViewID);
    if (Index == INDEX_NONE)
    {
        return;
    }

    Views.RemoveAt(Index);

    // Adjust current index if needed
    if (CurrentViewIndex >= Views.Num())
    {
        CurrentViewIndex = FMath::Max(0, Views.Num() - 1);
    }

    UpdateViewTitle();
    UpdateButtonStates();
}

FName URViewNavigationWidget::GetCurrentViewID() const
{
    if (CurrentViewIndex >= 0 && CurrentViewIndex < Views.Num())
    {
        return Views[CurrentViewIndex].ViewID;
    }
    return NAME_None;
}

FRViewEntry URViewNavigationWidget::GetCurrentView() const
{
    if (CurrentViewIndex >= 0 && CurrentViewIndex < Views.Num())
    {
        return Views[CurrentViewIndex];
    }
    return FRViewEntry();
}

bool URViewNavigationWidget::HasView(FName ViewID) const
{
    return GetViewIndex(ViewID) != INDEX_NONE;
}

int32 URViewNavigationWidget::GetViewIndex(FName ViewID) const
{
    for (int32 i = 0; i < Views.Num(); ++i)
    {
        if (Views[i].ViewID == ViewID)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

void URViewNavigationWidget::UpdateViewTitle()
{
    if (Txt_ViewTitle && CurrentViewIndex >= 0 && CurrentViewIndex < Views.Num())
    {
        Txt_ViewTitle->SetText(Views[CurrentViewIndex].DisplayName);
    }
}

void URViewNavigationWidget::UpdateButtonStates()
{
    if (!bEnableWrapping)
    {
        // Disable buttons at edges
        if (Btn_NavLeft)
        {
            Btn_NavLeft->SetIsEnabled(CurrentViewIndex > 0);
        }

        if (Btn_NavRight)
        {
            Btn_NavRight->SetIsEnabled(CurrentViewIndex < Views.Num() - 1);
        }
    }
    else
    {
        // Always enabled when wrapping
        if (Btn_NavLeft)
        {
            Btn_NavLeft->SetIsEnabled(Views.Num() > 1);
        }

        if (Btn_NavRight)
        {
            Btn_NavRight->SetIsEnabled(Views.Num() > 1);
        }
    }
}

void URViewNavigationWidget::HandleNavLeftClicked()
{
    NavigateLeft();
}

void URViewNavigationWidget::HandleNavRightClicked()
{
    NavigateRight();
}
