#include "MotionHandler.h"

#include "Channels/MovieSceneBoolChannel.h"
#include "Channels/MovieSceneChannel.h"
#include "Channels/MovieSceneChannelHandle.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"
#include "CoreFwd.h"
#include "Editor.h"
#include "Editor/Sequencer/Public/IKeyArea.h"
#include "Editor/Sequencer/Public/SequencerChannelTraits.h"
#include "Internationalization/Text.h"
#include "UObject/NameTypes.h"

#include <stdexcept>

MotionHandler::MotionHandler(const IKeyArea* KeyArea_, double DefaultScale_, ISequencer* Sequencer_, Mode SetValueMode_)
{
	Sequencer = Sequencer_;

	Scale = DefaultScale_;

	KeyArea = KeyArea_;

	PreviousValue = 0;

	IsFirstUpdate = true;

	FMovieSceneChannelHandle Channel = KeyArea->GetChannel();

	ChannelTypeName = Channel.GetChannelTypeName();

	if (ChannelTypeName == "MovieSceneFloatChannel")
	{
		FloatChannel = Channel.Cast<FMovieSceneFloatChannel>().Get();
	}
	else if (ChannelTypeName == "MovieSceneDoubleChannel")
	{
		DoubleChannel = Channel.Cast<FMovieSceneDoubleChannel>().Get();
	}
	else if (ChannelTypeName == "MovieSceneBoolChannel")
	{
		BoolChannel = Channel.Cast<FMovieSceneBoolChannel>().Get();
	}
	else if (ChannelTypeName == "MovieSceneIntegerChannel")
	{
		IntegerChannel = Channel.Cast<FMovieSceneIntegerChannel>().Get();
	}

	SetValueMode = SetValueMode_;
}
void MotionHandler::SetKey(FFrameNumber InTime, FVector2D InputVector)
{
	InTime.Value = InTime.Value + 1000;
	double valueToSet = 0;
	if (SetValueMode == X)
	{
		valueToSet = InputVector.X;
	}
	else if (SetValueMode == XInverted)
	{
		valueToSet = InputVector.X * -1;
	}
	else if (SetValueMode == Y)
	{
		valueToSet = InputVector.Y;
	}
	else if (SetValueMode == YInverted)
	{
		valueToSet = InputVector.Y * -1;
	}

	FMovieSceneChannelHandle Channel = KeyArea->GetChannel();

	if (ChannelTypeName == "MovieSceneFloatChannel")
	{
		valueToSet = valueToSet * Scale;
		valueToSet = (float) valueToSet;
		TMovieSceneChannelData<FMovieSceneFloatValue> ChannelData = FloatChannel->GetData();
		if (!IsFirstUpdate)
		{
			ChannelData.UpdateOrAddKey(InTime, FMovieSceneFloatValue((float) PreviousValue + valueToSet));
		}
		else
		{
			ChannelData.UpdateOrAddKey(InTime, FMovieSceneFloatValue((float) valueToSet));
			float evalResult;
			InTime--;
			FloatChannel->Evaluate(InTime, evalResult);
			PreviousValue = (double) evalResult;
		}
	}
	else if (ChannelTypeName == "MovieSceneDoubleChannel")
	{
		DoubleChannel = Channel.Cast<FMovieSceneDoubleChannel>().Get();
		valueToSet = valueToSet * Scale;
		valueToSet = (double) valueToSet;
		TMovieSceneChannelData<FMovieSceneDoubleValue> ChannelData = DoubleChannel->GetData();
		if (!IsFirstUpdate)
		{
			valueToSet += PreviousValue;
			ChannelData.UpdateOrAddKey(InTime, FMovieSceneDoubleValue(valueToSet));
			PreviousValue = valueToSet;
		}
		else
		{
			ChannelData.UpdateOrAddKey(InTime, FMovieSceneDoubleValue(valueToSet));
			IsFirstUpdate = false;
			InTime--;
			DoubleChannel->Evaluate(InTime, PreviousValue);
		}
	}
	else if (ChannelTypeName == "MovieSceneBoolChannel")
	{
		BoolChannel = Channel.Cast<FMovieSceneBoolChannel>().Get();
		/* not implemented for now */
	}
	else if (ChannelTypeName == "MovieSceneIntegerChannel")
	{
		IntegerChannel = Channel.Cast<FMovieSceneIntegerChannel>().Get();
		valueToSet = valueToSet * Scale;
		valueToSet = (int32) valueToSet;
		TMovieSceneChannelData<int> ChannelData = IntegerChannel->GetData();
		if (!IsFirstUpdate)
		{
			ChannelData.UpdateOrAddKey(InTime, (int32) PreviousValue + valueToSet);
		}
		else
		{
			ChannelData.UpdateOrAddKey(InTime, (int32) valueToSet);
			IsFirstUpdate = false;
			int32 evalResult;
			InTime--;
			IntegerChannel->Evaluate(InTime, evalResult);
			PreviousValue = (double) evalResult;
		}
	}
}
