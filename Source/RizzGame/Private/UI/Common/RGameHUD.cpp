// RGameHUD.cpp
#include "UI/Common/RGameHUD.h"

UPanelWidget* URGameHUD::GetLayer(ERUILayer Layer) const
{
	switch (Layer)
	{
	case ERUILayer::Exploration: return L_Exploration;
	case ERUILayer::Combat:     return L_Combat;
	case ERUILayer::Overlay:    return L_Overlay;
	case ERUILayer::Popup:      return L_Popup;
	default:                    return nullptr;
	}
}

void URGameHUD::SetLayerVisible(ERUILayer Layer, bool bVisible, ESlateVisibility VisType)
{
	if (UPanelWidget* Root = GetLayer(Layer))
	{
		Root->SetVisibility(bVisible ? VisType : ESlateVisibility::Collapsed);
	}
}

bool URGameHUD::IsLayerVisible(ERUILayer Layer) const
{
	if (const UPanelWidget* Root = GetLayer(Layer))
	{
		return Root->GetVisibility() != ESlateVisibility::Collapsed;
	}
	return false;
}
