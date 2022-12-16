#include "MotionHandler.h"

#include "Channels/MovieSceneBoolChannel.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"
#include "ControlRig.h"
#include "CoreFwd.h"
#include "Editor.h"
#include "EulerTransform.h"
#include "ISequencer.h"
#include "ISequencerModule.h"
#include "Internationalization/Text.h"
#include "Math/TransformNonVectorized.h"
#include "PluginsLsp/Animation/ControlRig/Source/ControlRig/Public/ControlRig.h"
#include "PluginsLsp/Animation/ControlRig/Source/ControlRig/Public/Sequencer/MovieSceneControlRigParameterTrack.h"
#include "RigVMCore/RigVM.h"
#include "RigVMCore/RigVMExternalVariable.h"
#include "Runtime/Core/Public/CoreFwd.h"
#include "Runtime/MovieScene/Public/Channels/MovieSceneChannelHandle.h"
#include "Runtime/MovieScene/Public/MovieSceneSection.h"
#include "Runtime/RigVM/Public/RigVMCore/RigVM.h"
#include "Runtime/RigVM/Public/RigVMCore/RigVMExternalVariable.h"
#include "Sequencer/MovieSceneControlRigParameterTrack.h"
#include "Sequencer/MovieSceneControlRigSpaceChannel.h"
#include "SequencerAddKeyOperation.h"
#include "SequencerKeyParams.h"
#include "SequencerWidgets/Public/ITimeSlider.h"
#include "Templates/SharedPointerInternals.h"
#include "UObject/NameTypes.h"
#include "Units/RigUnitContext.h"

#include <stdexcept>
#include <string>

MotionHandler::MotionHandler()
{
}
MotionHandler::MotionHandler(const IKeyArea* KeyArea_, double DefaultScale_, ISequencer* Sequencer_, UMovieScene* MovieScene_,
	UMovieSceneTrack* MovieSceneTrack_, FGuid ObjectFGuid_, enum Mode Mode_)
{
	Sequencer = Sequencer_;
	Scale = DefaultScale_;
	KeyArea = KeyArea_;
	PreviousValue = 0;
	IsFirstUpdate = true;
	MovieScene = MovieScene_;
	Mode = Mode_;

	ObjectFGuid = ObjectFGuid_;
	MovieSceneTrack = MovieSceneTrack_;

	MovieSceneControlRigParameterTrack = Cast<UMovieSceneControlRigParameterTrack>(MovieSceneTrack);

	if (IsValid(MovieSceneControlRigParameterTrack))
	{
		TArray<FName> currentControlSelectionArr = MovieSceneControlRigParameterTrack->GetControlRig()->CurrentControlSelection();
		if (currentControlSelectionArr.Num() > 0)
		{
			currentControlSelection = currentControlSelectionArr[0];
			UE_LOG(LogTemp, Warning, TEXT("current control selection is %s"), *currentControlSelection.ToString())
		}
	}

	FMovieSceneChannelHandle Channel = KeyArea->GetChannel();

	ChannelTypeName = Channel.GetChannelTypeName();

	UE_LOG(LogTemp, Warning, TEXT("channe type is %s"), *ChannelTypeName.ToString());

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
}
FVector GetVectorFromString(FString input)
{
	FString firstString = *input.Left(11).Right(8);
	FString secondString = *input.Right(10).Left(8);
	if (secondString[0] == '=')
	{
		secondString = *input.Right(9).Left(8);
	}
	UE_LOG(LogTemp, Warning, TEXT("first string is  %s"), *firstString);
	UE_LOG(LogTemp, Warning, TEXT("second string  is  %s"), *secondString);
	FVector output = FVector();
	output.X = FCString::Atof(*firstString);
	output.Y = FCString::Atof(*secondString);
	return output;
}
void MotionHandler::SetKey(FFrameNumber InTime, FVector2D InputVector)
{
	double valueToSet = 0;
	if (Mode == X)
	{
		valueToSet = InputVector.X;
	}
	else if (Mode == XInverted)
	{
		valueToSet = InputVector.X * -1;
	}
	else if (Mode == Y)
	{
		valueToSet = InputVector.Y;
	}
	else if (Mode == YInverted)
	{
		valueToSet = InputVector.Y * -1;
	}

	FMovieSceneChannelHandle Channel = KeyArea->GetChannel();

	if (ChannelTypeName == "MovieSceneFloatChannel")
	{
		UE_LOG(LogTemp, Warning, TEXT("float triggered"))
		FloatChannel = Channel.Cast<FMovieSceneFloatChannel>().Get();
		UE_LOG(LogTemp, Warning, TEXT("Scale value is %f"), Scale);
		valueToSet = valueToSet * Scale;
		valueToSet = (float) valueToSet;
		TMovieSceneChannelData<FMovieSceneFloatValue> ChannelData = FloatChannel->GetData();
		if (!IsFirstUpdate)
		{
			ChannelData.UpdateOrAddKey(InTime, FMovieSceneFloatValue(PreviousValue));
			valueToSet += PreviousValue;
			ChannelData.UpdateOrAddKey(InTime, FMovieSceneFloatValue(valueToSet));
			PreviousValue = valueToSet;
		}
		else
		{
			ChannelData.UpdateOrAddKey(InTime, FMovieSceneFloatValue((float) valueToSet));
			InTime--;
			float prevValue = 0;
			FloatChannel->Evaluate(InTime, prevValue);
			PreviousValue = (double) prevValue;
			IsFirstUpdate = false;
		}
		SyncControlRigWithChannelValue(InTime);
	}
	else if (ChannelTypeName == "MovieSceneDoubleChannel")
	{
		UE_LOG(LogTemp, Warning, TEXT("double triggered"))
		DoubleChannel = Channel.Cast<FMovieSceneDoubleChannel>().Get();
		valueToSet = valueToSet * Scale;
		valueToSet = (double) valueToSet;
		TMovieSceneChannelData<FMovieSceneDoubleValue> ChannelData = DoubleChannel->GetData();
		if (!IsFirstUpdate)
		{
			ChannelData.UpdateOrAddKey(InTime - 1000, FMovieSceneDoubleValue(PreviousValue));
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
		UE_LOG(LogTemp, Warning, TEXT("integer triggered"))

		IntegerChannel = Channel.Cast<FMovieSceneIntegerChannel>().Get();
		valueToSet = valueToSet * Scale;
		valueToSet = (int32) valueToSet;
		TMovieSceneChannelData<int> ChannelData = IntegerChannel->GetData();
		if (!IsFirstUpdate)
		{
			ChannelData.UpdateOrAddKey(InTime, (int32) PreviousValue + valueToSet);
			PreviousValue = valueToSet;
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
void MotionHandler::SyncControlRigWithChannelValue(FFrameNumber InTime)
{
	FMovieSceneChannelHandle Channel = KeyArea->GetChannel();
	if (IsValid(MovieSceneControlRigParameterTrack))
	{
		UControlRig* controlRig = MovieSceneControlRigParameterTrack->GetControlRig();
		FRigControlElement* controlElement = controlRig->FindControl(currentControlSelection);

		UE_LOG(LogTemp, Warning, TEXT("Control element name is %s"), *controlElement->GetDisplayName().ToString());
		UE_LOG(LogTemp, Warning, TEXT("Control element type is %d"), controlElement->Settings.ControlType);
		ERigControlType controlType = controlElement->Settings.ControlType;
		FRigControlValue controlValue = controlRig->GetControlValue(currentControlSelection);
		FRigControlValue controlValueMin = controlElement->Settings.MinimumValue;
		FRigControlValue controlValueMax = controlElement->Settings.MaximumValue;
		float valueOfChannel = 0;
		FloatChannel->Evaluate(InTime, valueOfChannel);
		FString ChannelDisplayText = Channel.GetMetaData()->DisplayText.ToString();
		if (controlType == ERigControlType::Float)
		{
			controlRig->SetControlValue(currentControlSelection, valueOfChannel, true, FRigControlModifiedContext(), true, true);
		}
		else if (controlType == ERigControlType::EulerTransform)
		{
			FEulerTransform eulerValue = controlValue.GetAsTransform(ERigControlType::EulerTransform, ERigControlAxis::X);
			UE::Math::TQuat<double> rotation = eulerValue.GetRotation();
			FRotator rotator = rotation.Rotator();
			UE_LOG(LogTemp, Warning, TEXT("rotator is %s"), *rotator.ToString());
			FVector location = eulerValue.GetLocation();
			FVector scale = eulerValue.GetScale3D();

			UE_LOG(LogTemp, Warning, TEXT("Trying to set value of channel %f"), valueOfChannel);

			UE_LOG(LogTemp, Warning, TEXT("it's euler transform"));

			/* ue log max and min values todo */
			if (ChannelDisplayText == "Location.X")
			{
				location.X = valueOfChannel;
				eulerValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Location.Y")
			{
				location.Y = valueOfChannel;
				eulerValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Location.Z")
			{
				location.Z = valueOfChannel;
				eulerValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Rotation.Roll")
			{
				rotator.Roll = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				eulerValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Rotation.Pitch")
			{
				rotator.Pitch = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				eulerValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Rotation.Yaw")
			{
				rotator.Yaw = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				eulerValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Scale.X")
			{
				scale.X = valueOfChannel;
				eulerValue.SetScale3D(scale);
			}
			else if (ChannelDisplayText == "Scale.Y")
			{
				scale.Y = valueOfChannel;
				eulerValue.SetScale3D(scale);
			}
			else if (ChannelDisplayText == "Scale.Z")
			{
				scale.Z = valueOfChannel;
				eulerValue.SetScale3D(scale);
			}
			eulerValue.FromFTransform(eulerValue);
			controlRig->SetControlValue(currentControlSelection, eulerValue, true, FRigControlModifiedContext(), true, true);
		}
		else if (controlType == ERigControlType::TransformNoScale)
		{
			FTransformNoScale nValue = controlValue.GetAsTransform(ERigControlType::TransformNoScale, ERigControlAxis::X);
			FVector location = nValue.Location;
			UE::Math::TQuat<double> rotation = nValue.Rotation;
			FRotator rotator = rotation.Rotator();
			UE_LOG(LogTemp, Warning, TEXT("it's transform no scale"));
			if (ChannelDisplayText == "Location.X")
			{
				location.X = valueOfChannel;
				nValue.Location = location;
			}
			else if (ChannelDisplayText == "Location.Y")
			{
				location.Y = valueOfChannel;
				nValue.Location = location;
			}
			else if (ChannelDisplayText == "Location.Z")
			{
				location.Z = valueOfChannel;
				nValue.Location = location;
			}
			else if (ChannelDisplayText == "Rotation.Roll")
			{
				rotator.Roll = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				nValue.Rotation = rotation;
			}
			else if (ChannelDisplayText == "Rotation.Pitch")
			{
				rotator.Pitch = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				nValue.Rotation = rotation;
			}
			else if (ChannelDisplayText == "Rotation.Yaw")
			{
				rotator.Yaw = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				nValue.Rotation = rotation;
			}
		}
		else if (controlType == ERigControlType::Transform)
		{
			FTransform eulerValue = controlValue.GetAsTransform(ERigControlType::Transform, ERigControlAxis::X);
			UE_LOG(LogTemp, Warning, TEXT("It's transform is %s"), *eulerValue.ToString());
			FVector location = eulerValue.GetLocation();
			UE::Math::TQuat<double> rotation = eulerValue.GetRotation();
			FVector scale = eulerValue.GetScale3D();
			FRotator rotator = rotation.Rotator();
			if (ChannelDisplayText == "Location.X")
			{
				location.X = valueOfChannel;
				eulerValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Location.Y")
			{
				location.Y = valueOfChannel;
				eulerValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Location.Z")
			{
				location.Z = valueOfChannel;
				eulerValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Rotation.Roll")
			{
				rotator.Roll = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				eulerValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Rotation.Pitch")
			{
				rotator.Pitch = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				eulerValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Rotation.Yaw")
			{
				rotator.Yaw = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				eulerValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Scale.X")
			{
				scale.X = valueOfChannel;
				eulerValue.SetScale3D(scale);
			}
			else if (ChannelDisplayText == "Scale.Y")
			{
				scale.Y = valueOfChannel;
				eulerValue.SetScale3D(scale);
			}
			else if (ChannelDisplayText == "Scale.Z")
			{
				scale.Z = valueOfChannel;
				eulerValue.SetScale3D(scale);
			}
			controlRig->SetControlValue(currentControlSelection, eulerValue, true, FRigControlModifiedContext(), true, true);
		}
		else if (controlType == ERigControlType::Vector2D)
		{
			FString eulerValue = controlValue.ToString<FVector2D>();
			UE_LOG(LogTemp, Warning, TEXT("It's Vector2D:  %s"), *controlValue.ToString<FVector2D>());
			UE_LOG(LogTemp, Warning, TEXT("It's Vector2D min :  %s"), *controlValueMin.ToString<FVector2D>());
			UE_LOG(LogTemp, Warning, TEXT("It's Vector2D max :  %s"), *controlValueMax.ToString<FVector2D>());

			FVector currentValue = GetVectorFromString(*eulerValue);
			FVector minValue = GetVectorFromString(*controlValueMin.ToString<FVector2D>());
			FVector maxValue = GetVectorFromString(*controlValueMax.ToString<FVector2D>());

			FVector2D vec = FVector2D();
			if (ChannelDisplayText == "X")
			{
				vec.X = float(valueOfChannel);
				vec.Y = float(currentValue.Y);
			}
			else if (ChannelDisplayText == "Y")
			{
				vec.X = float(currentValue.X);
				vec.Y = float(valueOfChannel);
			}
			controlRig->SetControlValue(currentControlSelection, vec, true, FRigControlModifiedContext(), true, true);
		}
	}
}
void MotionHandler::InitKeys()
{
	TRange<FFrameNumber> playbackRange = MovieScene->GetPlaybackRange();
	FFrameNumber lowerValue = playbackRange.GetLowerBoundValue();
	FFrameNumber highValue = playbackRange.GetUpperBoundValue();
	/* todo list */
}
double MotionHandler::GetValueFromTime(FFrameNumber InTime)
{
	if (ChannelTypeName == "MovieSceneFloatChannel")
	{
		float result = 0;
		FloatChannel->Evaluate(InTime, result);
		return result;
	}
	if (ChannelTypeName == "MovieSceneDoubleChannel")
	{
		double result = 0;
		DoubleChannel->Evaluate(InTime, result);
		return result;
	}
	if (ChannelTypeName == "MovieSceneIntegerChannel")
	{
		int32 result = 0;
		IntegerChannel->Evaluate(InTime, result);
		return result;
	}
	return double(0);
}
void MotionHandler::DeleteKeysWithin(TRange<FFrameNumber> InRange)
{
	TArray<FKeyHandle> KeyHandles = TArray<FKeyHandle>();
	TArray<FFrameNumber> frames = TArray<FFrameNumber>();
	if (ChannelTypeName == "MovieSceneFloatChannel")
	{
		FloatChannel->GetKeys(InRange, &frames, &KeyHandles);
		FloatChannel->DeleteKeys(TArrayView<const FKeyHandle>(KeyHandles));
	}
	else if (ChannelTypeName == "MovieSceneDoubleChannel")
	{
		DoubleChannel->GetKeys(InRange, &frames, &KeyHandles);
		DoubleChannel->DeleteKeys(TArrayView<const FKeyHandle>(KeyHandles));
	}
	else if (ChannelTypeName == "MovieSceneIntegerChannel")
	{
		IntegerChannel->GetKeys(InRange, &frames, &KeyHandles);
		IntegerChannel->DeleteKeys(TArrayView<const FKeyHandle>(KeyHandles));
	}
}
void MotionHandler::DeleteAllKeysFrom(FFrameNumber InTime)
{
	if (ChannelTypeName == "MovieSceneFloatChannel")
	{
		FloatChannel->DeleteKeysFrom(InTime, false);
	}
	else if (ChannelTypeName == "MovieSceneDoubleChannel")
	{
		DoubleChannel->DeleteKeysFrom(InTime, false);
	}
	else if (ChannelTypeName == "MovieSceneIntegerChannel")
	{
		IntegerChannel->DeleteKeysFrom(InTime, false);
	}
}
void MotionHandler::Optimize(TRange<FFrameNumber> InRange)
{
	FKeyDataOptimizationParams params = FKeyDataOptimizationParams();
	params.bAutoSetInterpolation = true;
	params.Range = InRange;
	params.Tolerance = 0.2;
	params.DisplayRate = FFrameRate();
	params.DisplayRate.Numerator = 24;
	params.DisplayRate.Denominator = 1;

	FFrameNumber highValue = InRange.GetUpperBoundValue();
	highValue.Value += 1000;

	float value = GetValueFromTime(highValue);
	UE_LOG(LogTemp, Warning, TEXT("high value key value is %d  "), value);

	SetKey(highValue, FVector2D(0, 0));

	UE_LOG(LogTemp, Warning, TEXT("upwer value us %d"), highValue.Value);
	InRange.SetUpperBoundValue(highValue);

	if (ChannelTypeName == "MovieSceneFloatChannel")
	{
		FloatChannel->Optimize(params);
	}
	else if (ChannelTypeName == "MovieSceneDoubleChannel")
	{
		DoubleChannel->Optimize(params);
	}
	else if (ChannelTypeName == "MovieSceneIntegerChannel")
	{
		IntegerChannel->Optimize(params);
	}
}
