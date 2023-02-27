// Fill out your copyright notice in the Description page of Project Settings.

#include "Accelerator.h"

#include "Channels/MovieSceneChannelProxy.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"

#include <cmath>

Accelerator::Accelerator(
	FMovieSceneFloatChannel* floatChannel, FMovieSceneDoubleChannel* doubleChannel, FMovieSceneIntegerChannel* integerChannel, TRange<FFrameNumber> range)
{
	Range = range;

	FloatChannel = floatChannel;
	IntegerChannel = integerChannel;

	DoubleChannel = doubleChannel;
	UpdateBackup();
}

Accelerator::~Accelerator()
{
}
void Accelerator::Accelerate(int value, FFrameNumber currentPosition)
{
	if (currentPosition < Range.GetLowerBoundValue())
	{
		return;
	}
	TArray<FFrameNumber> times;
	TArray<FKeyHandle> keys;
	TRange<FFrameNumber> range = Range;
	range.SetLowerBoundValue(currentPosition); // we need to start from current position and perform all operations from current position
	DoubleChannel->GetKeys(range, &times, &keys);
	for (FFrameNumber& time : times)
	{
		time.Value *= 1 + value * 0.01;
	}
	DoubleChannel->SetKeyTimes(keys, times);
}
int Accelerator::FindNearestKeyBy(
	FFrameNumber frame, TArray<FFrameNumber> keyTimes) const	// return key with that FFrameNumber or next key
{
	int index = -1;
	for (int i = 0; i < keyTimes.Num(); i++)
	{
		if (keyTimes[i] >= frame)
		{
			return i;
		}
	}
	return index;
}
void Accelerator::Reset(TRange<FFrameNumber> range = TRange<FFrameNumber>())
{
	Range = range;
	if (FloatChannel != nullptr)
	{
	}
	else if (DoubleChannel != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Keys backup num is %d"), keysBackup.Num());
		DoubleChannel->SetKeyTimes(keysBackup, framesBackup);
	}
	else if (IntegerChannel != nullptr)
	{
	}
}
void Accelerator::UpdateBackup(TRange<FFrameNumber> range)
{
	if (range.IsEmpty())
	{
		range = Range;
	}
	if (DoubleChannel != nullptr)
	{
		DoubleChannel->GetKeys(range, &framesBackup, &keysBackup);
	}
}