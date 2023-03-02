// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

#include "MotionEditor.h"

#include "Channels/MovieSceneChannelProxy.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"

MotionEditor::MotionEditor(FMovieSceneFloatChannel* floatChannel, FMovieSceneDoubleChannel* doubleChannel, FMovieSceneIntegerChannel* integerChannel)
{
	FloatChannel = floatChannel;
	DoubleChannel = doubleChannel;
	IntegerChannel = integerChannel;

	if (FloatChannel != nullptr)
	{
		FloatChannelDup = *FloatChannel;
	}
	else if (DoubleChannel != nullptr)
	{
		DoubleChannelDup = *DoubleChannel;
	}
	else if (IntegerChannel != nullptr)
	{
		IntegerChannelDup = *IntegerChannel;
	}
}

MotionEditor::~MotionEditor()
{
}

void MotionEditor::Edit(FFrameNumber InTime, double value)
{
	if (FloatChannel != nullptr)
	{
		float valueToSet = (float)value;

		float Eval;
		FloatChannelDup.Evaluate(InTime, Eval);
		valueToSet += Eval;

		TMovieSceneChannelData<FMovieSceneFloatValue> ChannelData = FloatChannel->GetData();
		ChannelData.UpdateOrAddKey(InTime, FMovieSceneFloatValue(valueToSet));

	}
	else if (DoubleChannel != nullptr)
	{
		double valueToSet = (double)value;
		double Eval;
		DoubleChannelDup.Evaluate(InTime, Eval);
		valueToSet += Eval;
		TMovieSceneChannelData<FMovieSceneDoubleValue> ChannelData = DoubleChannel->GetData();
		ChannelData.UpdateOrAddKey(InTime, FMovieSceneDoubleValue(valueToSet));
	}
	else if (IntegerChannel != nullptr)
	{
		int valueToSet = (int32)value;
		int Eval;
		IntegerChannelDup.Evaluate(InTime, Eval);
		valueToSet += Eval;
		TMovieSceneChannelData<int> ChannelData = IntegerChannel->GetData();
		ChannelData.UpdateOrAddKey(InTime, valueToSet);
	}
}
void MotionEditor::Reset(TRange<FFrameNumber> range)
{
	TArray<FFrameNumber> times;
	TArray<FKeyHandle> keys;

	if (FloatChannel != nullptr)
	{
		FloatChannel->GetKeys(range, &times, &keys);
		FloatChannel->DeleteKeys(keys);
	}
	else if (DoubleChannel != nullptr)
	{
		DoubleChannel->GetKeys(range, &times, &keys);
		DoubleChannel->DeleteKeys(keys);
	}
	else if (IntegerChannel != nullptr)
	{
		IntegerChannel->GetKeys(range, &times, &keys);
		IntegerChannel->DeleteKeys(keys);
	}
}
void MotionEditor::ReInit()
{
	if (FloatChannel != nullptr)
	{
		FloatChannelDup = *FloatChannel;
	}
	else if (DoubleChannel != nullptr)
	{
		DoubleChannelDup = *DoubleChannel;
	}
	else if (IntegerChannel != nullptr)
	{
		IntegerChannelDup = *IntegerChannel;
	}
}
