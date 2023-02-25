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
	IsFirstExecution = true;
	Reset();
}

Accelerator::~Accelerator()
{
}
void Accelerator::Accelerate(int value, FFrameNumber keyPosition)
{
	int keyIndex = FindNearestKeyBy(keyPosition);
	UE_LOG(LogTemp, Warning, TEXT("called accelerate %d"), keyIndex);
	FFrameNumber moveBy = HowMuchCanMove(keyIndex, value);
	UE_LOG(LogTemp, Warning, TEXT("can move by %d"), moveBy.Value);
	UpdateKey(keyIndex, moveBy);
	TotalMovement += moveBy;
}
FFrameNumber Accelerator::HowMuchCanMove(int keyIndex, FFrameNumber moveBy)
{
	if (moveBy > 0)
	{
		return Times[keyIndex] + moveBy;
	}
	else if (moveBy < 0)
	{
		if (Times.IsValidIndex(keyIndex - 1))
		{
			FFrameNumber frameWouldBe = Times[keyIndex] + moveBy;
			if (frameWouldBe < Times[keyIndex - 1])	   // if frame would be < than previous frame
			{
				FFrameNumber newMoveBy = Times[keyIndex] - Times[keyIndex - 1] -
										 5;	   // when we should move it by maximum available size to previous frame
				return Times[keyIndex] - newMoveBy;
			}
			else
			{
				return Times[keyIndex] + moveBy;
			}
		}
		else
		{
			return Times[keyIndex] + moveBy;
		}
	}
	return FFrameNumber();
}
void Accelerator::UpdateKey(int keyIndex, FFrameNumber time)
{
	if (FloatChannel != nullptr)
	{
	}
	else if (DoubleChannel != nullptr)
	{
		FMovieSceneDoubleValue value = DoubleValues[keyIndex];
		DoubleChannel->GetData().UpdateOrAddKey(time, value);
	}
	else if (IntegerChannel != nullptr)
	{
	}
}
int Accelerator::FindNearestKeyBy(FFrameNumber frame)	 // return key with that FFrameNumber or next key
{
	int index = -1;
	for (int i = 0; i < Times.Num(); i++)
	{
		if (Times[i].Value == frame.Value)
		{
			return i;
		}
		else
		{
			if (frame.Value > Times[i].Value)
			{
				if (Times.IsValidIndex(i + 1))
				{
					if (frame.Value < Times[i + 1].Value)
					{
						return i + 1;
					}
				}
				else
				{
					return i;	 // if it's the last frame
				}
			}
		}
	}
	return index;
}
void Accelerator::Reset()
{
	if (FloatChannel != nullptr)
	{
	}
	else if (DoubleChannel != nullptr)
	{
		Times = DoubleChannel->GetTimes();
		DoubleValues = DoubleChannel->GetValues();
		if (Times.Num() > 0)
		{
			FFrameNumber lower = Times[0];
			FFrameNumber high = Times.Last().Value;
			Range.SetLowerBound(lower);
			Range.SetUpperBound(high);
		}
	}
	else if (IntegerChannel != nullptr)
	{
	}
	TotalMovement = FFrameNumber();
}