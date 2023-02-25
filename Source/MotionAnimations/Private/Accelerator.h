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
	void Reinit();

private:
	int FindNearestKeyBy(FFrameNumber frame);
	void UpdateKey(int keyIndex, FFrameNumber time);
	FFrameNumber HowMuchCanMove(int keyIndex, FFrameNumber moveBy);
	void BackChannelToOriginalState();


	bool IsFirstExecution;
	int LatestIndexSetted;


	TRange<FFrameNumber> Range;

	TArray<FFrameNumber> Times;
	TArray<FMovieSceneDoubleValue> DoubleValues;
	TArray<FMovieSceneFloatValue> FloatValues;
	TArray<int const> IntValues;

	FFrameNumber PreviousMove;

	FMovieSceneFloatChannel* FloatChannel;
	FMovieSceneDoubleChannel* DoubleChannel;
	FMovieSceneIntegerChannel* IntegerChannel;
};