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
	void ChangeRange(TRange<FFrameNumber> range);
	void Reset(TRange<FFrameNumber> range);
	void Reinit(TRange<FFrameNumber> range);

private:
	int FindNearestKeyBy(FFrameNumber frame, TArray<FFrameNumber> keyTimes) const;
	FFrameNumber GetNewPosition(int keyIndex, int keyIndexInNewTimes, FFrameNumber moveBy) const;
	void BackChannelToOriginalState();

	bool IsFirstExecution;
	int CurrentKeyInOldTimes;

	int LatestKeyUpdatedInNewKeyTimes;

	TRange<FFrameNumber> Range;

	TArray<FFrameNumber> OldKeyTimes;
	TArray<FMovieSceneDoubleValue> OldDoubleValues;
	TArray<FMovieSceneFloatValue> OldFloatValues;
	TArray<FFrameNumber> newKeyTimes;
	TArray<int const> IntValues;

	FFrameNumber StartFrom;
	FFrameNumber PreviousMove;

	FMovieSceneFloatChannel* FloatChannel;
	FMovieSceneDoubleChannel* DoubleChannel;
	FMovieSceneIntegerChannel* IntegerChannel;
};