// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "HAL/Platform.h"
#include "Misc/Guid.h"

#include "MotionHandlerData.generated.h"

USTRUCT()
struct FMotionHandlerData
{
	GENERATED_BODY()

public:
	double Scale;
	FGuid ObjectFGuid;
	FString TrackName;
	int32 SectionRowIndex;
	FString ChannelTypeName;
	int32 ChannelIndex;
	bool IsValidData;
	enum class Mode : uint8
	{
		X,
		XInverted,
		Y,
		YInverted,
	};
	Mode SelectedMode;

	FString ControlSelection;

	FMotionHandlerData();
	FMotionHandlerData(FString FilePath);
	FMotionHandlerData(double Scale, FGuid ObjectFGuid, FString TrackName, int32 SectionRowIndex, FString ChannelTypeName,
		int32 ChannelIndex, enum Mode Mode_);

	bool Save();
};
