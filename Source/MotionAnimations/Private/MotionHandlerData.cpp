// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

#include "MotionHandlerData.h"

#include "Containers/UnrealString.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "MotionAnimations.h"
#include "Math/Range.h"


FMotionHandlerData::FMotionHandlerData()
{
	IsValidData = false;
}
FMotionHandlerData::FMotionHandlerData(FString FilePath)
{
	FString FileData = "";
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		IsValidData = false;
		return;
	}

	FFileHelper::LoadFileToString(FileData, *FilePath);

	if (FJsonObjectConverter::JsonObjectStringToUStruct(FileData, this, 0, 0))
	{
		IsValidData = true;
	}
}
FMotionHandlerData::FMotionHandlerData(double Scale_, FGuid ObjectFGuid_, FString TrackName_, int32 SectionRowIndex_,
	FString ChannelTypeName_, int32 ChannelIndex_, enum Mode Mode_, FString SequenceName_, FText CustomName_,
	FText ChannelDisplayText_, FString KeyAreaName_, FString TrackDisplayName_)
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
	KeyAreaName = KeyAreaName_;
	TrackDisplayName = TrackDisplayName_;
	SetIndex = -1;
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
	return ObjectFGuid.ToString() + "." + TrackName + "." + TrackDisplayName + "." + FString::FromInt(SectionRowIndex) + "." +
		   ChannelTypeName + "." + KeyAreaName + "." + FString::FromInt(ChannelIndex);
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
