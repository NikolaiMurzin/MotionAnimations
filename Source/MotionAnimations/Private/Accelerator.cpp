// Fill out your copyright notice in the Description page of Project Settings.

#include "Accelerator.h"

#include "Channels/MovieSceneChannelProxy.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"

Accelerator::Accelerator(
	FMovieSceneFloatChannel* floatChannel, FMovieSceneDoubleChannel* doubleChannel, FMovieSceneIntegerChannel* integerChannel)
{
	Range = TRange<FFrameNumber>();
	IsFirstExecution = true;
	LatestIndexSetted = -1;

	FloatChannel = floatChannel;
	IntegerChannel = integerChannel;

	DoubleChannel = doubleChannel;
	DoubleValues = TArray<FMovieSceneDoubleValue>();
	Times = TArray<FFrameNumber>();

	Reinit();
}

Accelerator::~Accelerator()
{
}
void Accelerator::Accelerate(int value, FFrameNumber keyPosition)
{
	int keyIndex = FindNearestKeyBy(keyPosition);
	if (keyIndex == LatestIndexSetted)
	{
		return;
	}
	FFrameNumber movePlus = FFrameNumber();
	movePlus.Value = value;
	FFrameNumber moveBy = HowMuchCanMove(keyIndex, movePlus); // FFrameNumber moveBy = keyPosition + value;
	UpdateKey(keyIndex, moveBy);
	PreviousMove += movePlus;
	LatestIndexSetted = keyIndex;
}
FFrameNumber Accelerator::HowMuchCanMove(int keyIndex, FFrameNumber moveBy)
{
	if (!Times.IsValidIndex(keyIndex))
	{
		FFrameNumber fn = FFrameNumber();
		fn.Value = 0;
		return fn;
	}
	if (moveBy >= 0)
	{
		return Times[keyIndex] + moveBy + PreviousMove;
	}
	else if (moveBy < 0)
	{
		if (Times.IsValidIndex(keyIndex - 1))
		{
			FFrameNumber max = Times[keyIndex - 1] + PreviousMove; // take latest installed value??? here we need to take not from Times but from actual FDoubleChannel;A
		}	// Get previous value, set value but maximum as previous value, not lower!!!
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("triger  third "));
			return Times[keyIndex];
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
		DoubleChannel->AutoSetTangents();
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
		if (Times.IsValidIndex(1))
		{
			DoubleChannel->DeleteKeysFrom(Times[1], false);
		}
	}
	else if (IntegerChannel != nullptr)
	{
	}
	PreviousMove.Value = 0;
}
void Accelerator::Reinit()
{
	if (FloatChannel != nullptr)
	{
	}
	else if (DoubleChannel != nullptr)
	{
		Times.Reset();
		Times.Append(DoubleChannel->GetTimes());
		DoubleValues.Reset();
		DoubleValues.Append(DoubleChannel->GetValues());
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
	PreviousMove = FFrameNumber();
	PreviousMove.Value = 0;
}
void Accelerator::BackChannelToOriginalState()
{
	if (FloatChannel != nullptr)
	{
	}
	else if (DoubleChannel != nullptr)
	{
		DoubleChannel->Reset();
		const TArray<FFrameNumber> times = TArray<FFrameNumber>(Times);
		const TArray<FMovieSceneDoubleValue> values = TArray<FMovieSceneDoubleValue>(DoubleValues);
		DoubleChannel->AddKeys(times, values);
	}
	else if (IntegerChannel != nullptr)
	{
	}
}