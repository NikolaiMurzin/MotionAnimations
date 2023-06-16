// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Channels/MovieSceneChannelProxy.h"
// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

#include "Channels/MovieSceneIntegerChannel.h"

/**
 * 
 */
class MotionEditor
{
public:
	MotionEditor(FMovieSceneFloatChannel* floatChannel = nullptr, FMovieSceneDoubleChannel* doubleChannel = nullptr, FMovieSceneIntegerChannel* integerChannel = nullptr);
	~MotionEditor();
	void Edit(FFrameNumber InTime, double value);
	void ReInit();
	void Reset(TRange<FFrameNumber> range);
private:
	FMovieSceneFloatChannel* FloatChannel;
	FMovieSceneDoubleChannel* DoubleChannel;
	FMovieSceneIntegerChannel* IntegerChannel;

	FMovieSceneFloatChannel FloatChannelDup;
	FMovieSceneDoubleChannel DoubleChannelDup;
	FMovieSceneIntegerChannel IntegerChannelDup;

};
