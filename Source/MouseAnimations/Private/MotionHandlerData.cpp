// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionHandlerData.h"

#include "Containers/UnrealString.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"

FMotionHandlerData::FMotionHandlerData()
{
	IsValidData = false;
}
FMotionHandlerData::FMotionHandlerData(FString FileData)
{
	FString FilePath = "";
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
