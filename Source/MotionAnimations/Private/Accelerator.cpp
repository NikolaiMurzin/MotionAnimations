// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

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
		for (int i = 0; i < times.Num(); i++)
		{
			if (i != 0) // if it's not first element;
			{
				int valueOfPrevious = times[i - 1].Value;
				int expectedValue = times[i].Value * (1 + value * 0.01);
				if (expectedValue < valueOfPrevious) // if value that we will set is lower than previous
				{
					times[i].Value = times[i - 1].Value + 10; // then set value of previous element + 10
				}
				else
				{
					times[i].Value *= 1 + value * 0.01;
				}
			}
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
	TArray<FFrameNumber> times;
	TArray<FKeyHandle> keys;

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