// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Channels/MovieSceneChannelProxy.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"

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
	int FindNearestKeyBy(FFrameNumber frame, TArray<FFrameNumber> keyTimes) const;

	TRange<FFrameNumber> Range;

	FMovieSceneFloatChannel* FloatChannel;
	FMovieSceneDoubleChannel* DoubleChannel;
	FMovieSceneIntegerChannel* IntegerChannel;

	TArray<FKeyHandle> keysBackup;
	TArray<FFrameNumber> framesBackup;
};