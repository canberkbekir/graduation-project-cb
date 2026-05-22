// Fill out your copyright notice in the Description page of Project Settings.

#include "RizzGameEditor.h"
#include "WorldView/RWorldViewEditorExtension.h"
#include "ToolMenus.h"

IMPLEMENT_MODULE(FRizzGameEditorModule, RizzGameEditor)

void FRizzGameEditorModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FRizzGameEditorModule::RegisterMenus));
}

void FRizzGameEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	if (UToolMenus::IsToolMenuUIEnabled())
	{
		UToolMenus::Get()->UnregisterOwner(this);
	}
}

void FRizzGameEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	FRWorldViewEditorExtension::Register();
}
