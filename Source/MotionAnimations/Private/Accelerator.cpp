// Fill out your copyright notice in the Description page of Project Settings.


#include "Accelerator.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"

Accelerator::Accelerator(FMovieSceneFloatChannel* floatChannel, FMovieSceneDoubleChannel* doubleChannel, FMovieSceneIntegerChannel* integerChannel)
{
	FloatChannel = floatChannel;
	DoubleChannel = doubleChannel;
	IntegerChannel = integerChannel;
}

Accelerator::~Accelerator()
{
}
void Accelerator::Accelerate(float value, FFrameNumber from)
{
	if (FloatChannel != nullptr)
	{
	}
	else if (DoubleChannel != nullptr)
	{
	}
	else if (IntegerChannel != nullptr)
	{
	}
}
void Accelerator::MoveKeys(FFrameNumber moveFrom, FFrameNumber moveBy)
{
	int index = FindNearestKeyBy(moveFrom); // it shouldn't move lower than previous key
	for (int i = 0; i < KeyTimes.Num(); i++)
	{
		if (i >= index) // we only should take elemne
		{
			if (moveBy > 0)
			{
				KeyTimes[i] += moveBy;
			}
			else if (moveBy < 0)
			{
				if (KeyTimes.IsValidIndex(i - 1))
				{
					FFrameNumber frameWouldBe = KeyTimes[i] + moveBy;
					if (KeyTimes[i - 1] < frameWouldBe) // if previous key < than frame time would be if we modify it
					{
						moveBy = KeyTimes[i] - KeyTimes[i - 1] + 5;
						KeyTimes[i] += moveBy;
					}
				}
				else
				{
					KeyTimes[i] += moveBy;
				}
			}
		}
	}
}
int Accelerator::FindNearestKeyBy(FFrameNumber frame) // return key with that FFrameNumber or next key
{
	int index = -1;
	for (int i = 0; i < KeyTimes.Num(); i++)
	{
		if (KeyTimes[i].Value == frame.Value)
		{
			return i;
		}
		else
		{
			if (frame.Value > KeyTimes[i].Value && frame.Value < KeyTimes[i + 1].Value)
			{
				return i + 1;
			}
		}
	}
	return index;
}
void Accelerator::Reset()
{
	if (FloatChannel != nullptr)
	{
		FloatChannel->GetKeys(Range, &KeyTimes, &Keys);
		TArrayView<const FFrameNumber> Times = FloatChannel->GetTimes();
		if (Times.Num() > 0)
		{
			Range.SetLowerBoundValue(Times[0]);
			Range.SetUpperBoundValue(Times.Last());
			FloatChannel->GetKeys(Range, &KeyTimes, &Keys);
		}
		
	}
	else if (DoubleChannel != nullptr)
	{
		DoubleChannel->GetKeys(Range, &KeyTimes, &Keys);
		TArrayView<const FFrameNumber> Times = DoubleChannel->GetTimes();
		if (Times.Num() > 0)
		{
			Range.SetLowerBoundValue(Times[0]);
			Range.SetUpperBoundValue(Times.Last().Value);
			DoubleChannel->GetKeys(Range, &KeyTimes, &Keys);
			MoveKeys(7000, 444);
			MoveKeys(1, -400);
			MoveKeys(1444, -999);
		}
	}
	else if (IntegerChannel != nullptr)
	{
		TArrayView<const FFrameNumber> Times = IntegerChannel->GetTimes();
		if (Times.Num() > 0)
		{
			Range.SetLowerBoundValue(Times[0]);
			Range.SetUpperBoundValue(Times.Last());
			IntegerChannel->GetKeys(Range, &KeyTimes, &Keys);
		}
	}
	LatestMultiply = 0;
}