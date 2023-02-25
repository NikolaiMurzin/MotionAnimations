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
	int FindNearestKeyBy(FFrameNumber frame);
	void PasteKeys();

	bool IsFirstExecution;

	void UpdateKey(int keyIndex, FFrameNumber time);
	FFrameNumber HowMuchCanMove(int keyIndex, FFrameNumber moveBy);

	TRange<FFrameNumber> Range;

	TArrayView<const FFrameNumber> Times;
	TArrayView<const FMovieSceneDoubleValue> DoubleValues;
	TArrayView<const FMovieSceneFloatValue> FloatValues;
	TArrayView<int const> IntValues;

	FFrameNumber TotalMovement;

	FMovieSceneFloatChannel* FloatChannel;
	FMovieSceneDoubleChannel* DoubleChannel;
	FMovieSceneIntegerChannel* IntegerChannel;
};