// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionHandlerData.h"

#include "Containers/UnrealString.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "MouseAnimations.h"

FMotionHandlerData::FMotionHandlerData()
{
	IsValidData = false;
}
FMotionHandlerData::FMotionHandlerData(FString FilePath)
{
	FString FileData = "";
	UE_LOG(LogTemp, Warning, TEXT("Trying to init MotionHandlerData with filepath %s"), *FilePath);
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("DID NOT FIND FILE"));
		IsValidData = false;
		return;
	}

	FFileHelper::LoadFileToString(FileData, *FilePath);

	if (FJsonObjectConverter::JsonObjectStringToUStruct(FileData, this, 0, 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("CONVERTED"));
		IsValidData = true;
	}
}
FMotionHandlerData::FMotionHandlerData(double Scale_, FGuid ObjectFGuid_, FString TrackName_, int32 SectionRowIndex_,
	FString ChannelTypeName_, int32 ChannelIndex_, enum Mode Mode_, FString SequenceName_, FText CustomName_,
	FText ChannelDisplayText_)
{
	Scale = Scale_;
	ObjectFGuid = ObjectFGuid_;
	TrackName = TrackName_;
	SectionRowIndex = SectionRowIndex_;
	ChannelTypeName = ChannelTypeName_;
	ChannelIndex = ChannelIndex_;
	SelectedMode = Mode_;
	SequenceName = SequenceName_;
	CurrentIndex = 1;
	CustomName = CustomName_;
	ChannelDisplayText = ChannelDisplayText_;
}
bool FMotionHandlerData::Save()
{
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(*this, JsonString);
	FString FilePath = GetFilePath();
	return FFileHelper::SaveStringToFile(
		FStringView(JsonString), *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get());
}
bool FMotionHandlerData::Delete()
{
	FString FilePath = GetFilePath();
	if (FPaths::ValidatePath(FilePath) && FPaths::FileExists(FilePath))
	{
		return IFileManager::Get().Delete(*FilePath);
	}
	return false;
}
FString FMotionHandlerData::GetName()
{
	return ObjectFGuid.ToString() + "." + TrackName + "." + FString::FromInt(SectionRowIndex) + "." + ChannelTypeName + "." +
		   FString::FromInt(ChannelIndex);
}

FString FMotionHandlerData::GetFilePath()
{
	FString First = FPaths::ProjectPluginsDir();
	FString PluginName_ = PluginName;
	FString Second = "Saved";
	FString Third = SequenceName;
	FString Fourth = GetName();
	return FPaths::Combine(First, PluginName, Second, Third, Fourth);
}
