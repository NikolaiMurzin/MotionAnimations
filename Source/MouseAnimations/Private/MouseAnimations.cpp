// Copyright Epic Games, Inc. All Rights Reserved.

#include "MouseAnimations.h"
#include "MouseAnimationsStyle.h"
#include "MouseAnimationsCommands.h"
#include "SMouseAnimationsMenu.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "SMain.h"

static const FName MouseAnimationsTabName("MouseAnimations");

#define LOCTEXT_NAMESPACE "FMouseAnimationsModule"

void FMouseAnimationsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FMouseAnimationsStyle::Initialize();
	FMouseAnimationsStyle::ReloadTextures();

	FMouseAnimationsCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FMouseAnimationsCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FMouseAnimationsModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMouseAnimationsModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MouseAnimationsTabName, FOnSpawnTab::CreateRaw(this, &FMouseAnimationsModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FMouseAnimationsTabTitle", "MouseAnimations"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FMouseAnimationsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FMouseAnimationsStyle::Shutdown();

	FMouseAnimationsCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MouseAnimationsTabName);
}

TSharedRef<SDockTab> FMouseAnimationsModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FMouseAnimationsModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("MouseAnimations.cpp"))
		);


	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SMain)
		];
}

void FMouseAnimationsModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MouseAnimationsTabName);
}

void FMouseAnimationsModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FMouseAnimationsCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FMouseAnimationsCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMouseAnimationsModule, MouseAnimations)
