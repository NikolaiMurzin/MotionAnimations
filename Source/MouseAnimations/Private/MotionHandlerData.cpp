// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionHandlerData.h"

#include "Containers/UnrealString.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"

FMotionHandlerData::FMotionHandlerData()
{
	IsValidData = false;
}
FMotionHandlerData::FMotionHandlerData(FString FilePath)
{
	FString FileData = "";
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
	FString ChannelTypeName_, int32 ChannelIndex_, enum Mode Mode_, FString SequenceName_)
{
	Scale = Scale_;
	ObjectFGuid = ObjectFGuid_;
	TrackName = TrackName_;
	SectionRowIndex = SectionRowIndex_;
	ChannelTypeName = ChannelTypeName_;
	ChannelIndex = ChannelIndex_;
	SelectedMode = Mode_;
	SequenceName = SequenceName_;
}
bool FMotionHandlerData::Save()
{
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(*this, JsonString);
	UE_LOG(LogTemp, Warning, TEXT("Json string is"), *JsonString);
	FString FilePath;
	return FFileHelper::SaveStringToFile(
		FStringView(JsonString), *GetFilePath(), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get());
}
FString FMotionHandlerData::GetFilePath()
{
	FString First = FPaths::ProjectPluginsDir();
	FString Second = SequenceName;
	FString Third = ObjectFGuid.ToString() + "." + TrackName + "." + FString::FromInt(SectionRowIndex) + "." + ChannelTypeName +
					"." + FString::FromInt(ChannelIndex);
	return FPaths::Combine(First, Second, Third);
}
