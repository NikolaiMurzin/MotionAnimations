// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

#pragma once

#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Misc/FrameNumber.h"

#include "KeyValues.generated.h"

USTRUCT()
struct FKeyValues
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<FFrameNumber> Times;
	UPROPERTY()
	TArray<FMovieSceneFloatValue> FloatValues;
	UPROPERTY()
	TArray<FMovieSceneDoubleValue> DoubleValues;
	UPROPERTY()
	TArray<int32> IntegerValues;
};
