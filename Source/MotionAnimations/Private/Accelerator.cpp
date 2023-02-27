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

	Reinit(Range);
}

Accelerator::~Accelerator()
{
}
void Accelerator::Accelerate(int value, FFrameNumber currentPosition)
{
	UE_LOG(LogTemp, Warning, TEXT("Value to set is %d"), value);
	auto alreadyDoneOnAllKeys = [&]() -> bool { return CurrentKeyInOldTimes >= OldKeyTimes.Num(); };
	if (alreadyDoneOnAllKeys())
	{
		return;
	}

	auto thereNoKeysStartFrom = [&](FFrameNumber compare) -> bool
	{
		if (newKeyTimes.Num() > 0)
		{
			return newKeyTimes.Last() <= compare;
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
		UE_LOG(LogTemp, Warning, TEXT("need to update new key"));

		FFrameNumber positionOfPreviousElement = newKeyTimes[nearestKeyInNewTimes - 1];

		FFrameNumber positionOfElementWeNeedToUpdate = newKeyTimes[nearestKeyInNewTimes];

		FFrameNumber positionToSet = GetNewPosition(CurrentKeyInOldTimes, FFrameNumber(value));

		auto updateExistKey = [&]()
		{
			auto replaceKey = [&](FFrameNumber oldPosition, FFrameNumber newPosition)
			{
				if (DoubleChannel != nullptr)
				{
					FFrameNumber deleteFrom = oldPosition;
					deleteFrom.Value -= 50;
					UE_LOG(LogTemp, Warning, TEXT("Trying to update position: %d"), deleteFrom.Value);
					DoubleChannel->DeleteKeysFrom(oldPosition, false);
					setNewKey(newPosition);
				}
			};
			replaceKey(positionOfElementWeNeedToUpdate, positionToSet);
			auto replaceKeyInNewKeyTimes = [&](FFrameNumber oldPosition, FFrameNumber newPosition)
			{
				for (int i = 0; i < newKeyTimes.Num(); i++)
				{
					if (newKeyTimes[i] == oldPosition)
					{
						newKeyTimes[i] = newPosition;
					}
				}
			};
			replaceKeyInNewKeyTimes(positionOfElementWeNeedToUpdate, positionToSet);
		};
	}
	else if (needToSetNewKey())
	{
		UE_LOG(LogTemp, Warning, TEXT("need to set new key"));
		FFrameNumber positionToSet = GetNewPosition(CurrentKeyInOldTimes, FFrameNumber(value));
		setNewKey(positionToSet);
		newKeyTimes.Add(positionToSet);
		CurrentKeyInOldTimes++;
	}
}
FFrameNumber Accelerator::GetNewPosition(int keyIndexInOldTimes, FFrameNumber moveBy) const
{
	auto noElementWithThatKeyInOldTimes = [&]() -> bool { return !OldKeyTimes.IsValidIndex(keyIndexInOldTimes); };
	if (noElementWithThatKeyInOldTimes())
	{
		return Range.GetLowerBoundValue();
	}

	FFrameNumber latestKeyPositionInNewTimes = Range.GetLowerBoundValue();
	if (newKeyTimes.Num() > 0)
	{
		latestKeyPositionInNewTimes = newKeyTimes.Last();
	}
	else
	{
		latestKeyPositionInNewTimes = OldKeyTimes[0]; // that's for init not from lower bound of range, but from first key of our times
	}

	FFrameNumber differenceInOldTimes = FFrameNumber();
	differenceInOldTimes.Value = 0;

	auto hasPreviousInOldTimes = [&]() -> bool { return OldKeyTimes.IsValidIndex(keyIndexInOldTimes - 1); };
	if (hasPreviousInOldTimes())
	{
		differenceInOldTimes = OldKeyTimes[keyIndexInOldTimes] - OldKeyTimes[keyIndexInOldTimes - 1];
	}
	if (moveBy > 0)
	{
		FFrameNumber newPosition = latestKeyPositionInNewTimes + differenceInOldTimes + moveBy;
		return newPosition;
	}
	if (moveBy < 0)
	{
		FFrameNumber newPosition = latestKeyPositionInNewTimes + differenceInOldTimes + moveBy;
		if (newPosition < latestKeyPositionInNewTimes)
		{
			auto getTheMostSmallerValueThatWeCanSet = [&]() -> FFrameNumber { return latestKeyPositionInNewTimes += 100; };
			UE_LOG(LogTemp, Warning, TEXT("newPosition is lower latestKeyPositionInNewTimes so %d"),
				getTheMostSmallerValueThatWeCanSet().Value);
			return getTheMostSmallerValueThatWeCanSet();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("newPosition is bigger latestKeyPositionInNewTimes  %d"), newPosition.Value);
			return newPosition;
		}
		UE_LOG(LogTemp, Warning, TEXT("Move by value is  %d"), moveBy.Value);
	}
	FFrameNumber newPosition = latestKeyPositionInNewTimes + differenceInOldTimes + moveBy;
	return newPosition;
}
void Accelerator::UpdateOrAddKey(int keyIndexInOldTimes, FFrameNumber time)
{
	if (FloatChannel != nullptr)
	{
	}
	else if (DoubleChannel != nullptr)
	{
		FMovieSceneDoubleValue value = OldDoubleValues[keyIndexInOldTimes];

		DoubleChannel->GetData().UpdateOrAddKey(time, value);
		DoubleChannel->AutoSetTangents();
	}
	else if (IntegerChannel != nullptr)
	{
	}
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
	FFrameNumber deleteKeysFrom = Range.GetLowerBoundValue();
	if (!range.IsEmpty())
	{
		deleteKeysFrom = range.GetLowerBoundValue();
	}
	Range = range;
	if (FloatChannel != nullptr)
	{
	}
	else if (DoubleChannel != nullptr)
	{
		if (OldKeyTimes.IsValidIndex(1))
		{
			DoubleChannel->DeleteKeysFrom(Range.GetLowerBoundValue(), false);
		}
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
					int secondindex = times.Num() - i;
					OldKeyTimes.Append(times.Slice(i, secondindex));
					OldDoubleValues.Append(values.Slice(i, values.Num() - i));
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