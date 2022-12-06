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

			TArray<FFrameNumber> frames = TArray<FFrameNumber>();
			frames.Add(InTime);
			TArray<FMovieSceneFloatValue> values = TArray<FMovieSceneFloatValue>();
			values.Add(FMovieSceneFloatValue(valueToSet));

			FloatChannel->Set(frames, values);

			float test = 0;
			FloatChannel->Evaluate(InTime, test);
		}
		else
		{
			TArray<FFrameNumber> frames = TArray<FFrameNumber>();
			frames.Add(InTime);
			TArray<FMovieSceneFloatValue> values = TArray<FMovieSceneFloatValue>();
			values.Add(FMovieSceneFloatValue(valueToSet));
			FloatChannel->Set(frames, values);

			float test = 0;
			FloatChannel->Evaluate(InTime, test);
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

			TArray<FFrameNumber> frames = TArray<FFrameNumber>();
			frames.Add(InTime);
			TArray<FMovieSceneDoubleValue> values = TArray<FMovieSceneDoubleValue>();
			values.Add(FMovieSceneDoubleValue(valueToSet));

			DoubleChannel->Set(frames, values);

			double test = 0;
			DoubleChannel->Evaluate(InTime, test);
		}
		else
		{
			TArray<FFrameNumber> frames = TArray<FFrameNumber>();
			frames.Add(InTime);
			TArray<FMovieSceneDoubleValue> values = TArray<FMovieSceneDoubleValue>();
			values.Add(FMovieSceneDoubleValue(valueToSet));

			DoubleChannel->Set(frames, values);

			double test = 0;
			DoubleChannel->Evaluate(InTime, test);
		}
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
