#include "MotionHandler.h"

#include "Channels/MovieSceneBoolChannel.h"
#include "Channels/MovieSceneChannel.h"
#include "Channels/MovieSceneChannelHandle.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"
#include "CoreFwd.h"
#include "Editor/Sequencer/Public/IKeyArea.h"
#include "Internationalization/Text.h"
#include "UObject/NameTypes.h"

#include <stdexcept>

MotionHandler::MotionHandler(const IKeyArea* KeyArea_, double DefaultScale_, Mode SetValueMode_)
{
	Scale = DefaultScale_;

	KeyArea = KeyArea_;

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
		TOptional<float> def = FloatChannel->GetDefault();
		if (def.IsSet())
		{
			valueToSet += def.GetValue();
			FloatChannel->SetDefault((float) valueToSet);
			float f = 0;
			FloatChannel->Evaluate(InTime, f);
		}
		else
		{
			FloatChannel->SetDefault((float) valueToSet);
		}
	}
	else if (ChannelTypeName == "MovieSceneDoubleChannel")
	{
		DoubleChannel = Channel.Cast<FMovieSceneDoubleChannel>().Get();
		valueToSet = valueToSet * Scale;
		valueToSet = (double) valueToSet;
		TOptional<double> def = DoubleChannel->GetDefault();
		if (def.IsSet())
		{
			valueToSet += def.GetValue();

			DoubleChannel->SetDefault(valueToSet);
		}
		else
		{
			DoubleChannel->SetDefault(valueToSet);
		}
		float f = 0;
		FloatChannel->Evaluate(InTime, f);
	}
	else if (ChannelTypeName == "MovieSceneBoolChannel")
	{
		BoolChannel = Channel.Cast<FMovieSceneBoolChannel>().Get();
	}
	else if (ChannelTypeName == "MovieSceneIntegerChannel")
	{
		valueToSet = valueToSet * Scale;
		TOptional<int32> def = IntegerChannel->GetDefault();
		if (def.IsSet())
		{
			valueToSet += def.GetValue();
			IntegerChannel->SetDefault((int32) valueToSet);
		}
		else
		{
			IntegerChannel->SetDefault((int32) valueToSet);
		}
	}
}
