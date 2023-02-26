// Fill out your copyright notice in the Description page of Project Settings.

#include "Accelerator.h"

#include "Channels/MovieSceneChannelProxy.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"

#include <cmath>

Accelerator::Accelerator(
	FMovieSceneFloatChannel* floatChannel, FMovieSceneDoubleChannel* doubleChannel, FMovieSceneIntegerChannel* integerChannel)
{
	Range = TRange<FFrameNumber>();
	IsFirstExecution = true;

	FloatChannel = floatChannel;
	IntegerChannel = integerChannel;

	DoubleChannel = doubleChannel;
	OldDoubleValues = TArray<FMovieSceneDoubleValue>();
	OldKeyTimes = TArray<FFrameNumber>();

	newKeyTimes = TArray<FFrameNumber>();

	CurrentKeyInOldTimes = 0;

	Reinit();
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

	auto thereNoKeysStartFrom = [&](FFrameNumber compare) -> bool {
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

	auto needToUpdateExistKey = [&]() -> bool { return !thereNoKeysStartFrom(currentPosition); };

	int nearestKeyInNewTimes = FindNearestKeyBy(currentPosition, newKeyTimes);

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
	UE_LOG(LogTemp, Warning, TEXT("Move by value is  %d"), moveBy.Value);
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
	UE_LOG(LogTemp, Warning, TEXT("new key times len is %d"), newKeyTimes.Num());
	UE_LOG(LogTemp, Warning, TEXT("new key times last element is %d"), latestKeyPositionInNewTimes.Value);

	FFrameNumber differenceInOldTimes = FFrameNumber();
	differenceInOldTimes.Value = 0;

	auto hasPreviousInOldTimes = [&]() -> bool { return OldKeyTimes.IsValidIndex(keyIndexInOldTimes - 1); };
	if (hasPreviousInOldTimes())
	{
		differenceInOldTimes = OldKeyTimes[keyIndexInOldTimes] - OldKeyTimes[keyIndexInOldTimes - 1];
	}
	if (moveBy > 0)
	{
		FFrameNumber newPosition = latestKeyPositionInNewTimes + moveBy + differenceInOldTimes;
		UE_LOG(LogTemp, Warning, TEXT("moveBy bigger than zero , soo %d"), newPosition.Value);
		return newPosition;
	}
	FFrameNumber newPosition = latestKeyPositionInNewTimes + moveBy + differenceInOldTimes;
	UE_LOG(LogTemp, Warning, TEXT("return plain latestKeyPositionInNewTimes coz moveBy was < 0 %d"), newPosition.Value);
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
void Accelerator::Reset()
{
	if (FloatChannel != nullptr)
	{
	}
	else if (DoubleChannel != nullptr)
	{
		if (OldKeyTimes.IsValidIndex(1))
		{
			DoubleChannel->DeleteKeysFrom(OldKeyTimes[1], false);
		}
	}
	else if (IntegerChannel != nullptr)
	{
	}
	newKeyTimes.Reset();
	TotalSum = 0;
	CurrentKeyInOldTimes = 0;
}
void Accelerator::Reinit()
{
	if (FloatChannel != nullptr)
	{
	}
	else if (DoubleChannel != nullptr)
	{
		OldKeyTimes.Reset();
		OldKeyTimes.Append(DoubleChannel->GetTimes());
		OldDoubleValues.Reset();
		OldDoubleValues.Append(DoubleChannel->GetValues());

		newKeyTimes.Reset();

		if (OldKeyTimes.Num() > 0)
		{
			FFrameNumber lower = OldKeyTimes[0];
			FFrameNumber high = OldKeyTimes.Last().Value;
			Range.SetLowerBound(lower);
			Range.SetUpperBound(high);
		}
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