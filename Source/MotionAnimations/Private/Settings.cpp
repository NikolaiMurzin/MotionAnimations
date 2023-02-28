// Fill out your copyright notice in the Description page of Project Settings.

#include "Settings.h"

#include <JsonObjectConverter.h>
#include <Misc/FileHelper.h>

FSettings::FSettings()
{
	FString FilePath = GetFilePath();

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		SetDefaultSettings();
		Save();
		return;
	}
	else
	{
		LoadSettingsFromDisk();
	}
}

FString FSettings::GetFilePath()
{
	FString First = FPaths::ProjectPluginsDir();
	FString PluginName_ = PluginName;
	FString DirName = "Settings";
	FString FileName = "Settings.json";
	return FPaths::Combine(First, PluginName, DirName, FileName);
}

void FSettings::SetDefaultSettings()
{
	Keys.Add(TTuple<FString, FString>("Activate", "F2"));
	Keys.Add(TTuple<FString, FString>("Delete item", "F11"));
	Keys.Add(TTuple<FString, FString>("Refresh sequencer", "Q"));
	Keys.Add(TTuple<FString, FString>("Start scaling", "R"));
	/*Keys.Add(TTuple<FString, FString>("Start edit", "W")); */
	Keys.Add(TTuple<FString, FString>("Start recording", "E"));
	Keys.Add(TTuple<FString, FString>("Stop recording and optimize", "D"));
	Keys.Add(TTuple<FString, FString>("Preview animation", "A"));
	Keys.Add(TTuple<FString, FString>("Save item", "F5"));
	Keys.Add(TTuple<FString, FString>("Load keys from item to sequencer", "F6"));
	Keys.Add(TTuple<FString, FString>("Select x axis", "Z"));
	Keys.Add(TTuple<FString, FString>("Select -x axis", "X"));
	Keys.Add(TTuple<FString, FString>("Select y axis", "C"));
	Keys.Add(TTuple<FString, FString>("Select -y axis", "V"));
	Keys.Add(TTuple<FString, FString>("Set upper bound of custom range", "Y"));
	Keys.Add(TTuple<FString, FString>("Set lower bound of custom range", "T"));
}
bool FSettings::Save()
{
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(*this, JsonString);
	FString FilePath = GetFilePath();
	return FFileHelper::SaveStringToFile(
		FStringView(JsonString), *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get());
}
bool FSettings::LoadSettingsFromDisk()
{
	FString FilePath = GetFilePath();
	FString FileData = "";
	FFileHelper::LoadFileToString(FileData, *FilePath);
	if (FJsonObjectConverter::JsonObjectStringToUStruct(FileData, this, 0, 0))
	{
		return true;
	}
	return false;
}
FSettings::~FSettings()
{
}
