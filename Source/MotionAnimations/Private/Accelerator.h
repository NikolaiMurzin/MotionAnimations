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
	Accelerator(FMovieSceneFloatChannel* _FloatChannel = nullptr, FMovieSceneDoubleChannel* _DoubleChannel = nullptr, FMovieSceneIntegerChannel* _IntegerChannel = nullptr);
	~Accelerator();
	void Accelerate(int value, FFrameNumber from);
	void ChangeRange(TRange<FFrameNumber> range);
	void Reset();

private:
	void MoveKeys(FFrameNumber moveFrom, FFrameNumber moveBy);
	void MoveKey(FFrameNumber moveFrom, FFrameNumber moveBy);
	int FindNearestKeyBy(FFrameNumber frame);
	void PasteKeys();

	TRange<FFrameNumber> Range;

	TArray<FKeyHandle> Keys;
	TArray<FFrameNumber> KeyTimes;
	

	float LatestMultiply;

	FMovieSceneFloatChannel* FloatChannel;
	FMovieSceneDoubleChannel* DoubleChannel;
	FMovieSceneIntegerChannel* IntegerChannel;

	FMovieSceneFloatChannel* FloatChannelDup;
	FMovieSceneDoubleChannel* DoubleChannelDup;
	FMovieSceneIntegerChannel* IntegerChannelDup;
};