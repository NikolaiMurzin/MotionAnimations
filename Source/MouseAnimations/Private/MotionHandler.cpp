#include "MotionHandler.h"

#include "Channels/MovieSceneBoolChannel.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"
#include "ControlRig.h"
#include "CoreFwd.h"
#include "Editor.h"
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

MotionHandler::MotionHandler(const IKeyArea* KeyArea_, double DefaultScale_, ISequencer* Sequencer_, UMovieScene* MovieScene_,
	UMovieSceneTrack* MovieSceneTrack_, FGuid ObjectFGuid_)
{
	Sequencer = Sequencer_;
	Scale = DefaultScale_;
	KeyArea = KeyArea_;
	PreviousValue = 0;
	IsFirstUpdate = true;
	MovieScene = MovieScene_;

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

	InitKeys();

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

	UMovieSceneSection* section = KeyArea->GetOwningSection();
	FMovieSceneChannelProxy& proxy = section->GetChannelProxy();
	channelIndex = proxy.FindIndex(ChannelTypeName, Channel.Get());
	UE_LOG(LogTemp, Warning, TEXT("Channel index is %d"), channelIndex);
}
void MotionHandler::SetKey(FFrameNumber InTime, FVector2D InputVector, Mode Mode)
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

	if (IsValid(MovieSceneControlRigParameterTrack))
	{
		UControlRig* controlRig = MovieSceneControlRigParameterTrack->GetControlRig();
		FRigControlElement* controlElement = controlRig->FindControl(currentControlSelection);

		UE_LOG(LogTemp, Warning, TEXT("Control element name is %s"), *controlElement->GetDisplayName().ToString());
		UE_LOG(LogTemp, Warning, TEXT("Control element type is %d"), controlElement->Settings.ControlType);
		ERigControlType controlType = controlElement->Settings.ControlType;
		FRigControlValue controlValue = controlRig->GetControlValue(currentControlSelection);
		float valueOfChannel = 0;
		FloatChannel->Evaluate(InTime, valueOfChannel);
		FString ChannelDisplayText = Channel.GetMetaData()->DisplayText.ToString();
		if (controlType == ERigControlType::Float)
		{
			controlRig->SetControlValue(currentControlSelection, valueOfChannel, true, FRigControlModifiedContext(), true, true);
		}
		else if (controlType == ERigControlType::EulerTransform)
		{
			FEulerTransform eValue = controlValue.GetAsTransform(ERigControlType::EulerTransform, ERigControlAxis::X);
			FVector location = eValue.GetLocation();
			UE::Math::TQuat<double> rotation = eValue.GetRotation();
			FVector scale = eValue.GetScale3D();
			UE_LOG(LogTemp, Warning, TEXT("it's euler transform"));
			if (ChannelDisplayText == "Location.X")
			{
				location.X = valueOfChannel;
				eValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Location.Y")
			{
				location.Y = valueOfChannel;
				eValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Location.Z")
			{
				location.Z = valueOfChannel;
				eValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Rotation.Roll")
			{
				rotation.X = valueOfChannel;
				eValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Rotation.Pitch")
			{
				rotation.Y = valueOfChannel;
				eValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Rotation.Yaw")
			{
				rotation.Z = valueOfChannel;
				eValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Scale.X")
			{
				scale.X = valueOfChannel;
				eValue.SetScale3D(scale);
			}
			else if (ChannelDisplayText == "Scale.Y")
			{
				scale.Y = valueOfChannel;
				eValue.SetScale3D(scale);
			}
			else if (ChannelDisplayText == "Scale.Z")
			{
				scale.Z = valueOfChannel;
				eValue.SetScale3D(scale);
			}
			controlRig->SetControlValue(currentControlSelection, eValue, true, FRigControlModifiedContext(), true, true);
		}
		else if (controlType == ERigControlType::TransformNoScale)
		{
			FTransformNoScale nValue = controlValue.GetAsTransform(ERigControlType::TransformNoScale, ERigControlAxis::X);
			FVector location = nValue.Location;
			UE::Math::TQuat<double> rotation = nValue.Rotation;
			UE_LOG(LogTemp, Warning, TEXT("it's transform no scale"))
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
				rotation.X = valueOfChannel;
				nValue.Rotation = rotation;
			}
			else if (ChannelDisplayText == "Rotation.Pitch")
			{
				rotation.Y = valueOfChannel;
				nValue.Rotation = rotation;
			}
			else if (ChannelDisplayText == "Rotation.Yaw")
			{
				rotation.Z = valueOfChannel;
				nValue.Rotation = rotation;
			}
		}
		else if (controlType == ERigControlType::Transform)
		{
			FTransform eValue = controlValue.GetAsTransform(ERigControlType::Transform, ERigControlAxis::X);
			UE_LOG(LogTemp, Warning, TEXT("It's transform is %s"), *eValue.ToString());
			FVector location = eValue.GetLocation();
			UE::Math::TQuat<double> rotation = eValue.GetRotation();
			FVector scale = eValue.GetScale3D();
			if (ChannelDisplayText == "Location.X")
			{
				location.X = valueOfChannel;
				eValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Location.Y")
			{
				location.Y = valueOfChannel;
				eValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Location.Z")
			{
				location.Z = valueOfChannel;
				eValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Rotation.Roll")
			{
				rotation.X = valueOfChannel;
				eValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Rotation.Pitch")
			{
				rotation.Y = valueOfChannel;
				eValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Rotation.Yaw")
			{
				rotation.Z = valueOfChannel;
				eValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Scale.X")
			{
				scale.X = valueOfChannel;
				eValue.SetScale3D(scale);
			}
			else if (ChannelDisplayText == "Scale.Y")
			{
				scale.Y = valueOfChannel;
				eValue.SetScale3D(scale);
			}
			else if (ChannelDisplayText == "Scale.Z")
			{
				scale.Z = valueOfChannel;
				eValue.SetScale3D(scale);
			}
			controlRig->SetControlValue(currentControlSelection, eValue, true, FRigControlModifiedContext(), true, true);
		}
		else if (controlType == ERigControlType::Vector2D)
		{
			FString eValue = controlValue.ToString<FVector2D>();
			UE_LOG(LogTemp, Warning, TEXT("It's Vector2D:  %s"), *controlValue.ToString<FVector2D>());
			FString firstString = *eValue.Left(11).Right(8);
			FString secondString = *eValue.Right(10).Left(8);
			if (secondString[0] == '=')
			{
				secondString = *eValue.Right(9).Left(8);
			}
			UE_LOG(LogTemp, Warning, TEXT("first string is  %s"), *firstString);
			UE_LOG(LogTemp, Warning, TEXT("second string  is  %s"), *secondString);
			float first = FCString::Atof(*firstString);
			float second = FCString::Atof(*secondString);
			UE_LOG(LogTemp, Warning, TEXT("first is  %f"), first);
			UE_LOG(LogTemp, Warning, TEXT("second is  %f"), second);
			UE_LOG(LogTemp, Warning, TEXT("value of channel is  %f"), valueOfChannel);
			FVector2D vec = FVector2D();
			if (ChannelDisplayText == "X")
			{
				vec.X = float(valueOfChannel);
				vec.Y = float(second);
			}
			else if (ChannelDisplayText == "Y")
			{
				vec.X = float(first);
				vec.Y = float(valueOfChannel);
			}
			controlRig->SetControlValue(currentControlSelection, vec, true, FRigControlModifiedContext(), true, true);
		}
		else
		{
			/* float valueOfChannel = 0;
			FloatChannel->Evaluate(InTime, valueOfChannel);
			controlRig->SetControlValue(currentControlSelection, valueOfChannel, true, FRigControlModifiedContext(), true, true); */
		}
	}

	/*UpdateUI(InTime);*/
	/*(IKeyArea* KeyArea_ = const_cast<IKeyArea*>(KeyArea);
	FFrameNumber frameNumber = InTime;
	frameNumber.Value = InTime.Value + 1000;
	FKeyHandle keyHandle = KeyArea_->AddOrUpdateKey(frameNumber, ObjectFGuid, *Sequencer);*/

	/*IKeyArea* KeyArea_ = const_cast<IKeyArea*>(KeyArea);
	TSharedRef<ISequencerTrackEditor> SequencerEditor =
		FSpawnTrackEditor::CreateTrackEditor(TSharedRef<ISequencer, ESPMode::ThreadSafe>(&*Sequencer));
	TArray<TSharedRef<IKeyArea>> keyAreas = TArray<TSharedRef<IKeyArea>>();
	keyAreas.Add(TSharedRef<IKeyArea>(KeyArea_));
	UE::Sequencer::FAddKeyOperation operation =
		UE::Sequencer::FAddKeyOperation::FromKeyAreas(&*SequencerEditor, TArrayView<TSharedRef<IKeyArea>>(keyAreas));
	std::cout << "some text";
	ISequencer& Sequencerr = *Sequencer;
	std::cout << "some text";
	operation.Commit(InTime, Sequencerr); */

	UE_LOG(LogTemp, Warning, TEXT("key area name is %s"), *KeyArea->GetName().ToString());
	UE_LOG(LogTemp, Warning, TEXT("channel index is %d"), Channel.GetChannelIndex());
	UE_LOG(LogTemp, Warning, TEXT("channel.GetMetadata()->Name is %s"), *Channel.GetMetaData()->Name.ToString());
	UE_LOG(LogTemp, Warning, TEXT("channel display text is %s"), *Channel.GetMetaData()->DisplayText.ToString());
	UE_LOG(LogTemp, Warning, TEXT("channel metadata group is %s"), *Channel.GetMetaData()->Group.ToString());
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

/*void MotionHandler::UpdateUI(FFrameNumber InTime)
{
	TSharedRef<ISequencerTrackEditor> SequencerEditor = FSpawnTrackEditor::CreateTrackEditor(TSharedRef<ISequencer>(Sequencer));

	using namespace UE::Sequencer;

	TArray<TSharedRef<IKeyArea>> ka = TArray<TSharedRef<IKeyArea>>();
	ka.Add(TSharedRef<IKeyArea>(const_cast<IKeyArea*>(KeyArea)));
	TArrayView<TSharedRef<IKeyArea>> ar = TArrayView<TSharedRef<IKeyArea>>(ka);

	FAddKeyOperation::FromKeyAreas(&*SequencerEditor, ar).Commit(InTime, *Sequencer);
}*/

