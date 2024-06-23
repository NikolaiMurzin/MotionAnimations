// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Channels/MovieSceneChannelProxy.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"
#include "Math/Range.h"



/**
 * 
 */
class Accelerator
{
public:
	Accelerator(FMovieSceneFloatChannel* _FloatChannel = nullptr, FMovieSceneDoubleChannel* _DoubleChannel = nullptr, FMovieSceneIntegerChannel* _IntegerChannel = nullptr, TRange<FFrameNumber> range = TRange<FFrameNumber>().Empty());
	~Accelerator();
	void Accelerate(int value, FFrameNumber from);
	void Reset(TRange<FFrameNumber> range);
	void UpdateBackup(TRange <FFrameNumber> range = TRange<FFrameNumber>());

private:
	TRange<FFrameNumber> Range;

	FMovieSceneFloatChannel* FloatChannel;
	FMovieSceneDoubleChannel* DoubleChannel;
	FMovieSceneIntegerChannel* IntegerChannel;

	TArray<FKeyHandle> keysBackup;
	TArray<FFrameNumber> framesBackup;
};