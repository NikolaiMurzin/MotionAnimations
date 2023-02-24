// Fill out your copyright notice in the Description page of Project Settings.

#include "Accelerator.h"

#include "Channels/MovieSceneChannelProxy.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"

Accelerator::Accelerator(
	FMovieSceneFloatChannel* floatChannel, FMovieSceneDoubleChannel* doubleChannel, FMovieSceneIntegerChannel* integerChannel)
{
	FloatChannel = floatChannel;
	DoubleChannel = doubleChannel;
	IntegerChannel = integerChannel;
	Range = TRange<FFrameNumber>();
	Reset();
}

Accelerator::~Accelerator()
{
}
void Accelerator::Accelerate(int value, FFrameNumber from)
{
	MoveKeys(from, value);
	PasteKeys();
}
void Accelerator::MoveKeys(FFrameNumber moveFrom, FFrameNumber moveBy)
{
	int nearestKeyIndex = FindNearestKeyBy(moveFrom);	 // it shouldn't move lower than previous key
	for (int i = 0; i < KeyTimes.Num(); i++)
	{
		if (i >= nearestKeyIndex)
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
					if (frameWouldBe < KeyTimes[i - 1])	  // if frame would be < than previous frame
					{
						FFrameNumber newMoveBy = KeyTimes[i] - KeyTimes[i - 1] - 5; // when we should move it by maximum available size to previous frame
						KeyTimes[i] -= newMoveBy;
					}
					else
					{
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
void Accelerator::MoveKey(FFrameNumber moveFrom, FFrameNumber moveBy)
{
	int keyIndex = FindNearestKeyBy(moveFrom);
}
void Accelerator::PasteKeys()
{
	if (FloatChannel != nullptr)
	{
	}
	else if (DoubleChannel != nullptr)
	{
		TArrayView<const FMovieSceneDoubleValue> values = DoubleChannel->GetValues();
		DoubleChannel->Reset();
		TMovieSceneChannelData DoubleChannelData = DoubleChannel->GetData();
		for (int i = 0; i < KeyTimes.Num(); i++)
		{
			if (KeyTimes.IsValidIndex(i) && values.IsValidIndex(i))
			{
				int time = KeyTimes[i].Value;
				FMovieSceneDoubleValue value = values[i];
				DoubleChannelData.UpdateOrAddKey(time, value);
			}
		}
	}
	else if (IntegerChannel != nullptr)
	{
	}
}
int Accelerator::FindNearestKeyBy(FFrameNumber frame)	 // return key with that FFrameNumber or next key
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
		TArrayView<const FFrameNumber> Times = FloatChannel->GetTimes();
		if (Times.Num() > 0)
		{
			FFrameNumber lower = Times[0];
			FFrameNumber high = Times.Last().Value;
			Range.SetLowerBound(lower);
			Range.SetUpperBound(high);
			FloatChannel->GetKeys(Range, &KeyTimes, &Keys);
		}
	}
	else if (DoubleChannel != nullptr)
	{
		TArrayView<const FFrameNumber> Times = DoubleChannel->GetTimes();
		if (Times.Num() > 0)
		{
			FFrameNumber lower = Times[0];
			FFrameNumber high = Times.Last().Value;
			Range.SetLowerBound(lower);
			Range.SetUpperBound(high);
			DoubleChannel->GetKeys(Range, &KeyTimes, &Keys);
		}
	}
	else if (IntegerChannel != nullptr)
	{
		TArrayView<const FFrameNumber> Times = IntegerChannel->GetTimes();
		if (Times.Num() > 0)
		{
			FFrameNumber lower = Times[0];
			FFrameNumber high = Times.Last().Value;
			Range.SetLowerBound(lower);
			Range.SetUpperBound(high);
			IntegerChannel->GetKeys(Range, &KeyTimes, &Keys);
		}
	}
	LatestMultiply = 0;
}