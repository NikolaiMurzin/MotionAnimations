// Copyright Epic Games, Inc. All Rights Reserved.

#include "MotionAnimations.h"

#include "LevelEditor.h"
#include "MotionAnimationsCommands.h"
#include "MotionAnimationsStyle.h"
#include "SMain.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

static const FName MotionAnimationsTabName("MotionAnimations");

#define LOCTEXT_NAMESPACE "FMotionAnimationsModule"

void FMotionAnimationsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FMotionAnimationsStyle::Initialize();
	FMotionAnimationsStyle::ReloadTextures();

	FMotionAnimationsCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(FMotionAnimationsCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FMotionAnimationsModule::PluginButtonClicked), FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMotionAnimationsModule::RegisterMenus));

	FGlobalTabmanager::Get()
		->RegisterNomadTabSpawner(MotionAnimationsTabName, FOnSpawnTab::CreateRaw(this, &FMotionAnimationsModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FMotionAnimationsTabTitle", "MotionAnimations"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FMotionAnimationsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FMotionAnimationsStyle::Shutdown();

	FMotionAnimationsCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MotionAnimationsTabName);
}

TSharedRef<SDockTab> FMotionAnimationsModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)[
		// Put your tab content here!
		SNew(SMain)];
}

void FMotionAnimationsModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MotionAnimationsTabName);
}

void FMotionAnimationsModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FMotionAnimationsCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry =
					Section.AddEntry(FToolMenuEntry::InitToolBarButton(FMotionAnimationsCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMotionAnimationsModule, MotionAnimations)
