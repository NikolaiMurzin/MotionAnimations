// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings.h"
#include <Misc/FileHelper.h>
#include <JsonObjectConverter.h>

FSettings::FSettings()
{
	FString FileData = "";
	FString FilePath = GetFilePath();
	
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		SetDefaultSettings();
		Save();
		return;
	}

	FFileHelper::LoadFileToString(FileData, *FilePath);

	if (FJsonObjectConverter::JsonObjectStringToUStruct(FileData, this, 0, 0))
	{
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
}
bool FSettings::Save()
{
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(*this, JsonString);
	FString FilePath = GetFilePath();
	return FFileHelper::SaveStringToFile(
		FStringView(JsonString), *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get());
}
FSettings::~FSettings()
{
}
