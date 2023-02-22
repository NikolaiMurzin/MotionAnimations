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
	FString TrackDisplayName;
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
	FText ChannelDisplayText;

	UPROPERTY()
	FString KeyAreaName;

	UPROPERTY()
	int32 CurrentIndex;
	UPROPERTY()
	double UpperBoundValue = 0;
	UPROPERTY()
	double LowerBoundValue = 0;
	UPROPERTY()
	TMap<int32, FKeyValues> KeyValues;

	FMotionHandlerData();
	FMotionHandlerData(FString FilePath);
	FMotionHandlerData(double Scale, FGuid ObjectFGuid, FString TrackName, int32 SectionRowIndex, FString ChannelTypeName,
		int32 ChannelIndex, enum Mode Mode_, FString SequenceName_, FText CustomName_, FText ChannelDisplayText,
		FString KeyAreaName_, FString TrackDisplayName_);

	FString GetName();
	FString GetFilePath();
	bool Save();
	bool Delete();
};
