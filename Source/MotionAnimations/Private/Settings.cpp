// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

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
	Keys.Add(TTuple<FString, FString>("Update motion handlers", "Q"));
	Keys.Add(TTuple<FString, FString>("Start scaling", "R"));
	Keys.Add(TTuple<FString, FString>("Populate", "P"));
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
	Keys.Add(TTuple<FString, FString>("select 1 set", "One"));
	Keys.Add(TTuple<FString, FString>("select 2 set", "Two"));
	Keys.Add(TTuple<FString, FString>("select 3 set", "Three"));
	Keys.Add(TTuple<FString, FString>("select 4 set", "Four"));
	Keys.Add(TTuple<FString, FString>("select 5 set", "Five"));
	Keys.Add(TTuple<FString, FString>("select 6 set", "Six"));
	Keys.Add(TTuple<FString, FString>("select 7 set", "Seven"));
	Keys.Add(TTuple<FString, FString>("select 8 set", "Eight"));
	Keys.Add(TTuple<FString, FString>("select 9 set", "Nine"));
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
