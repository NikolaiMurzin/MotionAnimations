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
	IsFirstExecution = true;

	FloatChannel = floatChannel;
	IntegerChannel = integerChannel;

	DoubleChannel = doubleChannel;
	OldDoubleValues = TArray<FMovieSceneDoubleValue>();
	OldKeyTimes = TArray<FFrameNumber>();

	newKeyTimes = TArray<FFrameNumber>();

	CurrentKeyInOldTimes = 0;
	LatestKeyUpdatedInNewKeyTimes = 0;

	Reinit(Range);
}

Accelerator::~Accelerator()
{
}
void Accelerator::Accelerate(int value, FFrameNumber currentPosition)
{
	if (currentPosition > Range.GetUpperBoundValue())
	{
		return;
	}
	auto thereNoKeysStartFrom = [&](FFrameNumber startFrom) -> bool
	{
		if (newKeyTimes.Num() > 0)
		{
			return newKeyTimes.Last() <= startFrom;
		}
		else
		{
			return true;
		}
	};

	auto needToSetNewKey = [&]() -> bool { return thereNoKeysStartFrom(currentPosition); };
	auto setNewKey = [&](FFrameNumber newPosition)
	{
		FMovieSceneDoubleValue valueToSet = OldDoubleValues[CurrentKeyInOldTimes];
		DoubleChannel->GetData().UpdateOrAddKey(newPosition, valueToSet);
		DoubleChannel->AutoSetTangents();
	};

	int nearestKeyInNewTimes = FindNearestKeyBy(currentPosition, newKeyTimes);

	auto needToUpdateExistKey = [&]() -> bool { return !thereNoKeysStartFrom(currentPosition) && nearestKeyInNewTimes != -1; };

	if (needToUpdateExistKey())
	{
		FFrameNumber nearestKeyTime = newKeyTimes[nearestKeyInNewTimes];
		FFrameNumber positionToSet = nearestKeyTime + value;
		if (positionToSet < currentPosition)
		{
			positionToSet = currentPosition + 10;
		}
		if (positionToSet > Range.GetUpperBoundValue())
		{
			positionToSet = Range.GetUpperBoundValue();
		}
		UE_LOG(LogTemp, Warning, TEXT("need to update new key"));


		TArray<FFrameNumber> frames = TArray<FFrameNumber>();
		TArray<FKeyHandle> keyHandles = TArray<FKeyHandle>();
		TRange<FFrameNumber> range = Range;

		range.SetLowerBound(newKeyTimes[nearestKeyInNewTimes - 1]);

		DoubleChannel->GetKeys(range, &frames, &keyHandles);
		DoubleChannel->DeleteKeys(keyHandles);

		DoubleChannel->GetData().UpdateOrAddKey(positionToSet, OldDoubleValues[CurrentKeyInOldTimes - 1]);
		UE_LOG(LogTemp, Warning, TEXT("updated %d"), positionToSet.Value);
		newKeyTimes[nearestKeyInNewTimes] = positionToSet.Value;
	}
	else if (needToSetNewKey())
	{
		UE_LOG(LogTemp, Warning, TEXT("Value to set is %d"), value);
		auto alreadyDoneOnAllKeys = [&]() -> bool { return CurrentKeyInOldTimes >= OldKeyTimes.Num(); };
		if (alreadyDoneOnAllKeys())
		{
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("need to set new key"));
		FFrameNumber positionToSet = GetNewPosition(CurrentKeyInOldTimes, nearestKeyInNewTimes, FFrameNumber(value));
		setNewKey(positionToSet);
		UE_LOG(LogTemp, Warning, TEXT("added %d"), positionToSet.Value);
		newKeyTimes.Add(positionToSet);
		CurrentKeyInOldTimes++;
	}
}
FFrameNumber Accelerator::GetNewPosition(int keyIndexInOldTimes, int keyIndexInNewTimes, FFrameNumber moveBy) const // this shall not be used in update exist key
{
	auto noElementWithThatKeyInOldTimes = [&]() -> bool { return !OldKeyTimes.IsValidIndex(keyIndexInOldTimes); };
	if (noElementWithThatKeyInOldTimes())
	{
		return Range.GetLowerBoundValue();
	}

	FFrameNumber keyPositionInNewTimes;
	if (newKeyTimes.IsValidIndex(keyIndexInNewTimes))
	{
		keyPositionInNewTimes = newKeyTimes[keyIndexInNewTimes];
	}
	else
	{
		if (OldKeyTimes.IsValidIndex(0))
		{
			keyPositionInNewTimes = OldKeyTimes[0];
		}
		else
		{
			keyPositionInNewTimes = Range.GetLowerBoundValue();
		}
	}

	FFrameNumber differenceInOldTimes = FFrameNumber();
	differenceInOldTimes.Value = 0;

	auto hasPreviousInOldTimes = [&]() -> bool { return OldKeyTimes.IsValidIndex(keyIndexInOldTimes - 1); };

	if (hasPreviousInOldTimes())
	{
		differenceInOldTimes = OldKeyTimes[keyIndexInOldTimes] - OldKeyTimes[keyIndexInOldTimes - 1];
	}

	FFrameNumber newPosition = keyPositionInNewTimes + differenceInOldTimes  + moveBy;

	if (moveBy > 0)
	{
		newPosition = keyPositionInNewTimes + differenceInOldTimes  + moveBy;
	}
	else if (moveBy < 0)
	{
		if (newPosition < keyPositionInNewTimes )
		{
			auto getTheMostSmallerValueThatWeCanSet = [&]() -> FFrameNumber { return keyPositionInNewTimes += 10; };
			UE_LOG(LogTemp, Warning, TEXT("newPosition is lower latestKeyPositionInNewTimes so %d"),
				getTheMostSmallerValueThatWeCanSet().Value);
			newPosition = getTheMostSmallerValueThatWeCanSet();
		}
	}
	if (newPosition > Range.GetUpperBoundValue())
	{
		return Range.GetUpperBoundValue();
	}
	return newPosition;
}
int Accelerator::FindNearestKeyBy(
	FFrameNumber frame, TArray<FFrameNumber> keyTimes) const	// return key with that FFrameNumber or next key
{
	int index = -1;
	for (int i = 0; i < keyTimes.Num(); i++)
	{
		if (keyTimes[i].Value == frame.Value)
		{
			return i;
		}
		else
		{
			if (frame.Value > keyTimes[i].Value)
			{
				if (keyTimes.IsValidIndex(i + 1))
				{
					if (frame.Value < keyTimes[i + 1].Value)
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
void Accelerator::Reset(TRange<FFrameNumber> range = TRange<FFrameNumber>())
{
	Range = range;
	FFrameNumber deleteKeysFrom = Range.GetLowerBoundValue();
	if (!Range.IsEmpty())
	{
		deleteKeysFrom = Range.GetLowerBoundValue();
	}
	if (FloatChannel != nullptr)
	{
	}
	else if (DoubleChannel != nullptr)
	{
		TArray<FFrameNumber> frames = TArray<FFrameNumber>();
		TArray<FKeyHandle> keyHandles = TArray<FKeyHandle>();
		DoubleChannel->GetKeys(Range, &frames, &keyHandles);
		DoubleChannel->DeleteKeys(keyHandles);
	}
	else if (IntegerChannel != nullptr)
	{
	}
	newKeyTimes.Reset();
	CurrentKeyInOldTimes = 0;
}
void Accelerator::Reinit(TRange<FFrameNumber> range)
{
	if (FloatChannel != nullptr)
	{
	}
	else if (DoubleChannel != nullptr)
	{
		Range = range;

		OldKeyTimes.Reset();
		OldDoubleValues.Reset();

		if (!range.IsEmpty())
		{
			TArrayView<const FFrameNumber> times = DoubleChannel->GetTimes();
			TArrayView<const FMovieSceneDoubleValue> values = DoubleChannel->GetValues();
			for (int i = 0; i < times.Num(); i++)
			{
				if (times[i].Value >= range.GetLowerBoundValue().Value)
				{
					int countToAdd = times.Num() - i;
					for (int l = i; l < times.Num(); l++)
					{
						if (times[l].Value >= range.GetUpperBoundValue().Value)
						{
							countToAdd = l;
							break;
						}
					}
					OldKeyTimes.Append(times.Slice(i, countToAdd - 1));
					OldDoubleValues.Append(values.Slice(i, countToAdd - 1));
					break;
				}
			}
		}
		else
		{
			OldKeyTimes.Append(DoubleChannel->GetTimes());
			OldDoubleValues.Append(DoubleChannel->GetValues());
		}

		newKeyTimes.Reset();
	}
	else if (IntegerChannel != nullptr)
	{
	}
}
void Accelerator::BackChannelToOriginalState()
{
	if (FloatChannel != nullptr)
	{
	}
	else if (DoubleChannel != nullptr)
	{
		DoubleChannel->Reset();
		const TArray<FFrameNumber> times = TArray<FFrameNumber>(OldKeyTimes);
		const TArray<FMovieSceneDoubleValue> values = TArray<FMovieSceneDoubleValue>(OldDoubleValues);
		DoubleChannel->AddKeys(times, values);
	}
	else if (IntegerChannel != nullptr)
	{
	}
}