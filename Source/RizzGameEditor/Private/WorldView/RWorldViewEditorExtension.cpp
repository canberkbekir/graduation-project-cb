// RWorldViewEditorExtension.cpp
#include "WorldView/RWorldViewEditorExtension.h"
#include "LevelEditor.h"
#include "Editor.h"
#include "Engine/PostProcessVolume.h"
#include "EngineUtils.h"
#include "Styling/AppStyle.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

bool FRWorldViewEditorExtension::bNetworkWorldActive = false;

void FRWorldViewEditorExtension::Register()
{
	RegisterMenus();
}

void FRWorldViewEditorExtension::RegisterMenus()
{
	FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	TSharedPtr<FExtender> Extender = MakeShareable(new FExtender());
	Extender->AddToolBarExtension(
		"Play",
		EExtensionHook::After,
		nullptr,
		FToolBarExtensionDelegate::CreateStatic(&FRWorldViewEditorExtension::AddToolbarButton)
	);

	LevelEditor.GetToolBarExtensibilityManager()->AddExtender(Extender);
	UE_LOG(LogTemp, Warning, TEXT("Network World toolbar button registered via LevelEditorModule extender"));
}

void FRWorldViewEditorExtension::AddToolbarButton(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateStatic(&FRWorldViewEditorExtension::ToggleNetworkWorld),
			FCanExecuteAction(),
			FIsActionChecked::CreateStatic(&FRWorldViewEditorExtension::IsNetworkWorldActive)
		),
		NAME_None,
		NSLOCTEXT("RizzEditor", "ToggleNetworkWorld", "Net World"),
		NSLOCTEXT("RizzEditor", "ToggleNetworkWorldTip", "Preview Network World post process in the viewport"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.PostProcessVolume"),
		EUserInterfaceActionType::ToggleButton
	);
}

void FRWorldViewEditorExtension::ToggleNetworkWorld()
{
	bNetworkWorldActive = !bNetworkWorldActive;
	SetPostProcessEnabled(bNetworkWorldActive);
	UE_LOG(LogTemp, Warning, TEXT("Network World toggled: %s"), bNetworkWorldActive ? TEXT("Enabled") : TEXT("Disabled"));
}

bool FRWorldViewEditorExtension::IsNetworkWorldActive()
{
	return bNetworkWorldActive;
}

void FRWorldViewEditorExtension::SetPostProcessEnabled(bool bEnabled)
{
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if (!EditorWorld)
	{
		return;
	}

	for (TActorIterator<APostProcessVolume> It(EditorWorld); It; ++It)
	{
		if (It->ActorHasTag(FName("NetworkWorld")))
		{
			It->bEnabled = bEnabled;
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Network World toggle: no PostProcessVolume with tag 'NetworkWorld' found in level."));
}
