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
	auto changetimes = [&]() {
		for (FFrameNumber& time : times)
		{
			time.Value *= 1 + value * 0.01;
		}
	};
	if (DoubleChannel != nullptr)
	{
		DoubleChannel->GetKeys(range, &times, &keys);
		changetimes();
		DoubleChannel->SetKeyTimes(keys, times);
	}
	else if (FloatChannel != nullptr)
	{
		FloatChannel->GetKeys(range, &times, &keys);
		changetimes();
		FloatChannel->SetKeyTimes(keys, times);
	}
	else if (IntegerChannel != nullptr)
	{
		IntegerChannel->GetKeys(range, &times, &keys);
		changetimes();
		IntegerChannel->SetKeyTimes(keys, times);
	}
}
void Accelerator::Reset(TRange<FFrameNumber> range = TRange<FFrameNumber>())
{
	Range = range;
	if (FloatChannel != nullptr)
	{
		FloatChannel->SetKeyTimes(keysBackup, framesBackup);
	}
	else if (DoubleChannel != nullptr)
	{
		DoubleChannel->SetKeyTimes(keysBackup, framesBackup);
	}
	else if (IntegerChannel != nullptr)
	{
		IntegerChannel->SetKeyTimes(keysBackup, framesBackup);
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
	else if(FloatChannel != nullptr)
	{
		FloatChannel->GetKeys(range, &framesBackup, &keysBackup);
	}
	else if (IntegerChannel != nullptr)
	{
		IntegerChannel->GetKeys(range, &framesBackup, &keysBackup);
	} 
}