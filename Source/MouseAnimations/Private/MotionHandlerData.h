// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "HAL/Platform.h"
#include "KeyValues.h"
#include "Misc/Guid.h"
#include "MotionHandlerMode.h"

#include "MotionHandlerData.generated.h"

USTRUCT()
struct FMotionHandlerData
{
	GENERATED_BODY()
public:
	UPROPERTY()
	double Scale;
	UPROPERTY()
	FGuid ObjectFGuid;
	UPROPERTY()
	FString TrackName;
	UPROPERTY()
	int32 SectionRowIndex;
	UPROPERTY()
	FString ChannelTypeName;
	UPROPERTY()
	int32 ChannelIndex;

	UPROPERTY()
	FString ControlSelection;
	UPROPERTY()
	FString SequenceName;
	bool IsValidData;
	UPROPERTY()
	Mode SelectedMode;
	UPROPERTY()
	FText CustomName;

	UPROPERTY()
	int32 CurrentIndex;
	UPROPERTY()
	TArray<FKeyValues> KeyValues;

	FMotionHandlerData();
	FMotionHandlerData(FString FilePath);
	FMotionHandlerData(double Scale, FGuid ObjectFGuid, FString TrackName, int32 SectionRowIndex, FString ChannelTypeName,
		int32 ChannelIndex, enum Mode Mode_, FString SequenceName_);

	FString GetName();
	FString GetFilePath();
	bool Save();
	bool Delete();
};
