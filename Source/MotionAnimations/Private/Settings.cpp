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
	Keys.Add(TTuple<FString, FString>("Activate", "F3"));
	Keys.Add(TTuple<FString, FString>("Delete item", "F11"));
	Keys.Add(TTuple<FString, FString>("Update motion handlers", "Q"));
	// Keys.Add(TTuple<FString, FString>("Start scaling", "R"));
	Keys.Add(TTuple<FString, FString>("Populate", "P"));
	Keys.Add(TTuple<FString, FString>("Start recording", "E"));
	Keys.Add(TTuple<FString, FString>("Stop recording", "D"));
	Keys.Add(TTuple<FString, FString>("Preview animation", "A"));
	Keys.Add(TTuple<FString, FString>("Save item", "F5"));
	Keys.Add(TTuple<FString, FString>("Load keys from item to sequencer", "F6"));
	Keys.Add(TTuple<FString, FString>("Select x axis", "X"));
	Keys.Add(TTuple<FString, FString>("Select -x axis", "C"));
	Keys.Add(TTuple<FString, FString>("Select y axis", "V"));
	Keys.Add(TTuple<FString, FString>("Select -y axis", "B"));
	Keys.Add(TTuple<FString, FString>("Set upper bound of custom range", "Y"));
	Keys.Add(TTuple<FString, FString>("Set lower bound of custom range", "T"));
	Keys.Add(TTuple<FString, FString>("Optimize", "G"));
	Keys.Add(TTuple<FString, FString>("select 1 set", "NumPadOne"));
	Keys.Add(TTuple<FString, FString>("select 2 set", "NumPadTwo"));
	Keys.Add(TTuple<FString, FString>("select 3 set", "NumPadThree"));
	Keys.Add(TTuple<FString, FString>("select 4 set", "NumPadFour"));
	Keys.Add(TTuple<FString, FString>("select 5 set", "NumPadFive"));
	Keys.Add(TTuple<FString, FString>("select 6 set", "NumPadSix"));
	Keys.Add(TTuple<FString, FString>("select 7 set", "NumPadSeven"));
	Keys.Add(TTuple<FString, FString>("select 8 set", "NumPadEight"));
	Keys.Add(TTuple<FString, FString>("select 9 set", "NumPadNine"));
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
