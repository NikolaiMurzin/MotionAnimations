#include "MotionHandler.h"

#include "Channels/MovieSceneBoolChannel.h"
#include "Channels/MovieSceneChannelProxy.h"
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
#include "KeyValues.h"
#include "Math/NumericLimits.h"
#include "Math/TransformNonVectorized.h"
#include "MotionHandlerData.h"
#include "MotionHandlerMode.h"
#include "MovieSceneSequence.h"
#include "MovieSceneTrack.h"
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
#include "UObject/Object.h"
#include "Units/RigUnitContext.h"

#include <stdexcept>
#include <string>

MotionHandler::MotionHandler(TSharedPtr<ISequencer> Sequencer_, UMovieSceneSequence* Sequence, FString FilePath)
{
	Data = FMotionHandlerData(FilePath);
	Sequencer = Sequencer_;

	OnScaleValueChanged.BindRaw(this, &MotionHandler::OnScaleValueChangedRaw);
	OnTextChanged.BindRaw(this, &MotionHandler::OnTextChangedRaw);

	OnCurrentIndexValueChanged.BindRaw(this, &MotionHandler::OnCurrentIndexValueChangedRaw);

	PreviousValue = 0;
	IsFirstUpdate = true;

	if (Sequence == nullptr)
	{
		return;
	}
	MovieScene = Sequence->GetMovieScene();
	if (MovieScene == nullptr)
	{
		return;
	}
	FMovieSceneBinding* Binding = MovieScene->FindBinding(Data.ObjectFGuid);
	if (Binding == nullptr)
	{
		return;
	}
	for (UMovieSceneTrack* Track_ : Binding->GetTracks())
	{
		if (Track_->GetTrackName().ToString() == Data.TrackName)
		{
			MovieSceneTrack = Track_;
			break;
		}
	}
	UMovieSceneSection* section;
	if (MovieSceneTrack == nullptr)
	{
		return;
	}
	for (UMovieSceneSection* Section_ : MovieSceneTrack->GetAllSections())
	{
		if (Section_->GetRowIndex() == Data.SectionRowIndex)
		{
			section = Section_;
			break;
		}
	}
	if (section == nullptr)
	{
		return;
	}
	FMovieSceneChannelProxy& channelProxy = section->GetChannelProxy();
	ChannelHandle = channelProxy.MakeHandle(FName(Data.ChannelTypeName), Data.ChannelIndex);
	if (ChannelHandle.Get() == nullptr)
	{
		return;
	}

	SetControlRigTrack(MovieSceneTrack);
	CastChannel();
	InitKeys();
}
MotionHandler::MotionHandler(const IKeyArea* KeyArea_, double Scale, TSharedPtr<ISequencer> Sequencer_,
	UMovieSceneSequence* Sequence_, UMovieSceneTrack* MovieSceneTrack_, FGuid ObjectFGuid_, Mode Mode_)
{
	Sequencer = Sequencer_;
	KeyArea = KeyArea_;
	PreviousValue = 0;
	IsFirstUpdate = true;
	MovieScene = Sequence_->GetMovieScene();
	MovieSceneTrack = MovieSceneTrack_;

	OnScaleValueChanged.BindRaw(this, &MotionHandler::OnScaleValueChangedRaw);
	OnTextChanged.BindRaw(this, &MotionHandler::OnTextChangedRaw);
	OnCurrentIndexValueChanged.BindRaw(this, &MotionHandler::OnCurrentIndexValueChangedRaw);

	FString TrackName_ = MovieSceneTrack_->GetTrackName().ToString();
	int32 RowIndex = KeyArea->GetOwningSection()->GetRowIndex();
	FString ChannelTypeName = KeyArea->GetChannel().GetChannelTypeName().ToString();
	int32 ChannelIndex = KeyArea->GetChannel().GetChannelIndex();

	ChannelHandle = KeyArea->GetChannel();
	ChannelDisplayText = ChannelHandle.GetMetaData()->DisplayText.ToString();
	Data = FMotionHandlerData(Scale, ObjectFGuid_, TrackName_, RowIndex, ChannelTypeName, ChannelIndex, Mode_,
		Sequence_->GetDisplayName().ToString(), FText::FromString(ChannelDisplayText));
	SetControlRigTrack(MovieSceneTrack);
	CastChannel();
	InitKeys();
}

bool MotionHandler::IsValidMotionHandler()
{
	UE_LOG(LogTemp, Warning, TEXT("Is valid Motion handler called"));
	if (Sequencer == nullptr || MovieScene == nullptr || MovieSceneTrack == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Motion handler is not valid"));
		ValidMotionHandler = false;
		return false;
	}
	bool IsBindingExistInSequencer = Sequencer->FindObjectsInCurrentSequence(Data.ObjectFGuid).Num() > 0;
	bool ChannelNullPtr =
		(FloatChannel == nullptr || DoubleChannel == nullptr || BoolChannel == nullptr || IntegerChannel == nullptr);
	bool IsValid_ = !ChannelNullPtr || !IsBindingExistInSequencer;
	if (!IsValid_)
	{
		UE_LOG(LogTemp, Warning, TEXT("Motion handler is not valid"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Motion handler is valid"));
	}
	ValidMotionHandler = IsValid_;
	return IsValid_;
}

void MotionHandler::SetControlRigTrack(UMovieSceneTrack* MovieSceneTrack_)
{
	MovieSceneControlRigParameterTrack = Cast<UMovieSceneControlRigParameterTrack>(MovieSceneTrack_);

	if (IsValid(MovieSceneControlRigParameterTrack))
	{
		TArray<FName> currentControlSelectionArr = MovieSceneControlRigParameterTrack->GetControlRig()->CurrentControlSelection();
		if (currentControlSelectionArr.Num() > 0)
		{
			Data.ControlSelection = currentControlSelectionArr[0].ToString();
		}
	}
}
void MotionHandler::CastChannel()
{
	if (Data.ChannelTypeName == "MovieSceneFloatChannel")
	{
		FloatChannel = ChannelHandle.Cast<FMovieSceneFloatChannel>().Get();
	}
	else if (Data.ChannelTypeName == "MovieSceneDoubleChannel")
	{
		DoubleChannel = ChannelHandle.Cast<FMovieSceneDoubleChannel>().Get();
	}
	else if (Data.ChannelTypeName == "MovieSceneBoolChannel")
	{
		BoolChannel = ChannelHandle.Cast<FMovieSceneBoolChannel>().Get();
	}
	else if (Data.ChannelTypeName == "MovieSceneIntegerChannel")
	{
		IntegerChannel = ChannelHandle.Cast<FMovieSceneIntegerChannel>().Get();
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
	FVector output = FVector();
	output.X = FCString::Atof(*firstString);
	output.Y = FCString::Atof(*secondString);
	return output;
}
void MotionHandler::SetKey(FFrameNumber InTime, FVector2D InputVector)
{
	if (!ValidMotionHandler)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't set key, motion handler is not valid"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Set key called"));
	double valueToSet = 0;
	if (Data.SelectedMode == Mode::X)
	{
		valueToSet = InputVector.X;
	}
	else if (Data.SelectedMode == Mode::XInverted)
	{
		valueToSet = InputVector.X * -1;
	}
	else if (Data.SelectedMode == Mode::Y)
	{
		valueToSet = InputVector.Y;
	}
	else if (Data.SelectedMode == Mode::YInverted)
	{
		valueToSet = InputVector.Y * -1;
	}

	if (Data.ChannelTypeName == "MovieSceneFloatChannel")
	{
		valueToSet = valueToSet * Data.Scale;
		valueToSet = (float) valueToSet;
		if (!FloatChannel->HasAnyData())
		{
			UE_LOG(LogTemp, Warning, TEXT("Can't get data of float channel"));
			return;
		}
		TMovieSceneChannelData<FMovieSceneFloatValue> ChannelData = FloatChannel->GetData();
		if (!IsFirstUpdate)
		{
			valueToSet += PreviousValue;
			UE_LOG(LogTemp, Warning, TEXT("Update or add key called"));
			ChannelData.UpdateOrAddKey(InTime, FMovieSceneFloatValue(valueToSet));
			UE_LOG(LogTemp, Warning, TEXT("After Update or add key called"));
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
	else if (Data.ChannelTypeName == "MovieSceneDoubleChannel")
	{
		valueToSet = valueToSet * Data.Scale;
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
	else if (Data.ChannelTypeName == "MovieSceneBoolChannel")
	{
		BoolChannel = ChannelHandle.Cast<FMovieSceneBoolChannel>().Get();
		/* not implemented for now */
	}
	else if (Data.ChannelTypeName == "MovieSceneIntegerChannel")
	{
		valueToSet = valueToSet * Data.Scale;
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
	if (IsValid(MovieSceneControlRigParameterTrack))
	{
		UControlRig* controlRig = MovieSceneControlRigParameterTrack->GetControlRig();
		FRigControlElement* controlElement = controlRig->FindControl(FName(Data.ControlSelection));
		UE_LOG(LogTemp, Warning, TEXT("Trying to set value to control %s"), *Data.ControlSelection);
		ERigControlType controlType = controlElement->Settings.ControlType;
		FRigControlValue controlValue = controlRig->GetControlValue(FName(Data.ControlSelection));
		FRigControlValue controlValueMin = controlElement->Settings.MinimumValue;
		FRigControlValue controlValueMax = controlElement->Settings.MaximumValue;
		float valueOfChannel = 0;
		FloatChannel->Evaluate(InTime, valueOfChannel);
		if (controlType == ERigControlType::Float)
		{
			controlRig->SetControlValue(
				FName(Data.ControlSelection), valueOfChannel, true, FRigControlModifiedContext(), true, true);
		}
		else if (controlType == ERigControlType::EulerTransform)
		{
			FEulerTransform eulerValue;
			eulerValue.FromFTransform(controlValue.GetAsTransform(ERigControlType::EulerTransform, ERigControlAxis::X));
			UE::Math::TQuat<double> rotation = eulerValue.GetRotation();
			FRotator rotator = rotation.Rotator();
			FVector location = eulerValue.GetLocation();
			FVector scale = eulerValue.GetScale3D();

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
			controlRig->SetControlValue(FName(Data.ControlSelection), eulerValue, true, FRigControlModifiedContext(), true, true);
		}
		else if (controlType == ERigControlType::TransformNoScale)
		{
			FTransformNoScale nValue = controlValue.GetAsTransform(ERigControlType::TransformNoScale, ERigControlAxis::X);
			FVector location = nValue.Location;
			UE::Math::TQuat<double> rotation = nValue.Rotation;
			FRotator rotator = rotation.Rotator();
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
			controlRig->SetControlValue(FName(Data.ControlSelection), eulerValue, true, FRigControlModifiedContext(), true, true);
		}
		else if (controlType == ERigControlType::Vector2D)
		{
			FString eulerValue = controlValue.ToString<FVector2D>();

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
			controlRig->SetControlValue(FName(Data.ControlSelection), vec, true, FRigControlModifiedContext(), true, true);
		}
	}
}
void MotionHandler::InitKeys()
{
	if (IsValidMotionHandler())
	{
		return;
	}
	TRange<FFrameNumber> playbackRange = MovieScene->GetPlaybackRange();
	FFrameNumber lowerValue = playbackRange.GetLowerBoundValue();
	FFrameNumber highValue = playbackRange.GetUpperBoundValue();
	/* todo list */
	FFrameNumber currentFrame = Sequencer->GetGlobalTime().Time.GetFrame();
	float value = GetValueFromTime(currentFrame);

	FFrameNumber nextFrame = Sequencer->GetGlobalTime().Time.GetFrame();
	nextFrame.Value += 1000;
	float secondValue = GetValueFromTime(currentFrame);
	if (Data.ChannelTypeName == "MovieSceneFloatChannel")
	{
		FloatChannel->GetData().UpdateOrAddKey(currentFrame, FMovieSceneFloatValue(value));
		FloatChannel->GetData().UpdateOrAddKey(nextFrame, FMovieSceneFloatValue(secondValue));
	}
	else if (Data.ChannelTypeName == "MovieSceneDoubleChannel")
	{
		DoubleChannel->GetData().UpdateOrAddKey(currentFrame, FMovieSceneDoubleValue(value));
		DoubleChannel->GetData().UpdateOrAddKey(nextFrame, FMovieSceneDoubleValue(secondValue));
	}
	else if (Data.ChannelTypeName == "MovieSceneIntegerChannel")
	{
		IntegerChannel->GetData().UpdateOrAddKey(currentFrame, int32(value));
		IntegerChannel->GetData().UpdateOrAddKey(nextFrame, int32(secondValue));
	}
}
double MotionHandler::GetValueFromTime(FFrameNumber InTime)
{
	if (!IsValidMotionHandler())
	{
		return double(0);
	}
	UE_LOG(LogTemp, Warning, TEXT("Get value from time called"));
	if (Data.ChannelTypeName == "MovieSceneFloatChannel")
	{
		if (FloatChannel != nullptr && FloatChannel->HasAnyData())
		{
			float result = 0;
			FloatChannel->Evaluate(InTime, result);
			UE_LOG(LogTemp, Warning, TEXT("Get value from time returned"));
			return result;
		}
	}
	if (Data.ChannelTypeName == "MovieSceneDoubleChannel")
	{
		if (DoubleChannel != nullptr && DoubleChannel->HasAnyData())
		{
			double result = 0;
			DoubleChannel->Evaluate(InTime, result);
			UE_LOG(LogTemp, Warning, TEXT("Get value from time returned"));
			return result;
		}
	}
	if (Data.ChannelTypeName == "MovieSceneIntegerChannel")
	{
		if (IntegerChannel != nullptr && IntegerChannel->HasAnyData())
		{
			int32 result = 0;
			IntegerChannel->Evaluate(InTime, result);
			UE_LOG(LogTemp, Warning, TEXT("Get value from time returned"));
			return result;
		}
	}
	return double(0);
}
void MotionHandler::DeleteKeysWithin(TRange<FFrameNumber> InRange)
{
	if (!IsValidMotionHandler())
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Delete keys within called"));
	TArray<FKeyHandle> KeyHandles = TArray<FKeyHandle>();
	TArray<FFrameNumber> frames = TArray<FFrameNumber>();
	if (Data.ChannelTypeName == "MovieSceneFloatChannel")
	{
		if (FloatChannel != nullptr)
		{
			FloatChannel->GetKeys(InRange, &frames, &KeyHandles);
			FloatChannel->DeleteKeys(TArrayView<const FKeyHandle>(KeyHandles));
		}
	}
	else if (Data.ChannelTypeName == "MovieSceneDoubleChannel")
	{
		if (DoubleChannel != nullptr)
		{
			DoubleChannel->GetKeys(InRange, &frames, &KeyHandles);
			DoubleChannel->DeleteKeys(TArrayView<const FKeyHandle>(KeyHandles));
		}
	}
	else if (Data.ChannelTypeName == "MovieSceneIntegerChannel")
	{
		if (IntegerChannel != nullptr)
		{
			IntegerChannel->GetKeys(InRange, &frames, &KeyHandles);
			IntegerChannel->DeleteKeys(TArrayView<const FKeyHandle>(KeyHandles));
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Delete keys within returned"));
}
void MotionHandler::DeleteAllKeysFrom(FFrameNumber InTime)
{
	if (Data.ChannelTypeName == "MovieSceneFloatChannel")
	{
		FloatChannel->DeleteKeysFrom(InTime, false);
	}
	else if (Data.ChannelTypeName == "MovieSceneDoubleChannel")
	{
		DoubleChannel->DeleteKeysFrom(InTime, false);
	}
	else if (Data.ChannelTypeName == "MovieSceneIntegerChannel")
	{
		IntegerChannel->DeleteKeysFrom(InTime, false);
	}
}
void MotionHandler::Optimize(TRange<FFrameNumber> InRange)
{
	UE_LOG(LogTemp, Warning, TEXT("Optimize Called"));
	if (!IsValidMotionHandler())
	{
		return;
	}
	FKeyDataOptimizationParams params = FKeyDataOptimizationParams();
	params.bAutoSetInterpolation = true;
	params.Range = InRange;
	params.Tolerance = 0.1;
	params.DisplayRate = FFrameRate();
	params.DisplayRate.Numerator = 24;
	params.DisplayRate.Denominator = 1;

	FFrameNumber highValue = InRange.GetUpperBoundValue();
	highValue.Value += 1000;

	float value = 0;
	value = GetValueFromTime(highValue);

	SetKey(highValue, FVector2D(value, value));

	InRange.SetUpperBoundValue(highValue);

	if (Data.ChannelTypeName == "MovieSceneFloatChannel")
	{
		if (FloatChannel != nullptr)
		{
			FloatChannel->Optimize(params);
		}
	}
	else if (Data.ChannelTypeName == "MovieSceneDoubleChannel")
	{
		if (DoubleChannel != nullptr)
		{
			DoubleChannel->Optimize(params);
		}
	}
	else if (Data.ChannelTypeName == "MovieSceneIntegerChannel")
	{
		if (IntegerChannel != nullptr)
		{
			IntegerChannel->Optimize(params);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Optimize returned"));
}
FGuid MotionHandler::GetObjectFGuid()
{
	return Data.ObjectFGuid;
}
void MotionHandler::SetSelectedMode(Mode Mode_)
{
	Data.SelectedMode = Mode_;
}
Mode MotionHandler::GetSelectedMode()
{
	return Data.SelectedMode;
}
bool MotionHandler::SaveData()
{
	return Data.Save();
}
bool MotionHandler::DeleteData()
{
	return Data.Delete();
}
void MotionHandler::AddNewKeyValues()
{
	if (!IsValidMotionHandler())
	{
		return;
	}
	TRange<FFrameNumber> playbackRange = MovieScene->GetPlaybackRange();
	if (Data.ChannelTypeName == "MovieSceneFloatChannel")
	{
		FKeyValues KeyValues = FKeyValues();
		KeyValues.FloatValues = FloatChannel->GetValues();
		KeyValues.Times = FloatChannel->GetTimes();
		Data.CurrentIndex = Data.KeyValues.Add(KeyValues);
	}
	else if (Data.ChannelTypeName == "MovieSceneDoubleChannel")
	{
		FKeyValues KeyValues = FKeyValues();
		KeyValues.DoubleValues = DoubleChannel->GetValues();
		KeyValues.Times = DoubleChannel->GetTimes();
		Data.CurrentIndex = Data.KeyValues.Add(KeyValues);
	}
	else if (Data.ChannelTypeName == "MovieSceneIntegerChannel")
	{
		FKeyValues KeyValues = FKeyValues();
		KeyValues.IntegerValues = IntegerChannel->GetValues();
		KeyValues.Times = IntegerChannel->GetTimes();
		Data.CurrentIndex = Data.KeyValues.Add(KeyValues);
	}
}
void MotionHandler::AddOrUpdateKeyValueInSequencer()
{
	if (!IsValidMotionHandler())
	{
		return;
	}
	if (!Data.KeyValues.IsValidIndex(Data.CurrentIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Add new key values!"));
		AddNewKeyValues();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Update key values!"));
		UpdateCurrentKeyValues();
	}
}
void MotionHandler::UpdateCurrentKeyValues()
{
	if (!IsValidMotionHandler())
	{
		return;
	}
	if (!Data.KeyValues.IsValidIndex(Data.CurrentIndex))
	{
		return;
	}
	TRange<FFrameNumber> playbackRange = MovieScene->GetPlaybackRange();
	if (Data.ChannelTypeName == "MovieSceneFloatChannel")
	{
		FKeyValues KeyValues = FKeyValues();
		KeyValues.FloatValues = FloatChannel->GetValues();
		KeyValues.Times = FloatChannel->GetTimes();
		Data.KeyValues[Data.CurrentIndex] = KeyValues;
	}
	else if (Data.ChannelTypeName == "MovieSceneDoubleChannel")
	{
		FKeyValues KeyValues = FKeyValues();
		KeyValues.DoubleValues = DoubleChannel->GetValues();
		KeyValues.Times = DoubleChannel->GetTimes();
		Data.KeyValues[Data.CurrentIndex] = KeyValues;
	}
	else if (Data.ChannelTypeName == "MovieSceneIntegerChannel")
	{
		FKeyValues KeyValues = FKeyValues();
		KeyValues.IntegerValues = IntegerChannel->GetValues();
		KeyValues.Times = IntegerChannel->GetTimes();
		Data.KeyValues[Data.CurrentIndex] = KeyValues;
	}
}
void MotionHandler::DeleteKeyValues()
{
	if (!IsValidMotionHandler())
	{
		return;
	}
	if (Data.KeyValues.IsValidIndex(Data.CurrentIndex))
	{
		Data.KeyValues.RemoveAtSwap(Data.CurrentIndex);
	}
}
void MotionHandler::InsertCurrentKeyValuesToSequencer()
{
	if (!Data.KeyValues.IsValidIndex(Data.CurrentIndex) || !IsValidMotionHandler())
	{
		return;
	}
	if (MovieScene != nullptr)
	{
		TRange<FFrameNumber> playbackRange = MovieScene->GetPlaybackRange();
		DeleteKeysWithin(playbackRange);
		if (Data.ChannelTypeName == "MovieSceneFloatChannel")
		{
			TArray<FFrameNumber> Times = Data.KeyValues[Data.CurrentIndex].Times;
			TArray<FMovieSceneFloatValue> Values = Data.KeyValues[Data.CurrentIndex].FloatValues;
			if (FloatChannel != nullptr)
			{
				FloatChannel->Set(Times, Values);
			}
		}
		else if (Data.ChannelTypeName == "MovieSceneDoubleChannel")
		{
			TArray<FFrameNumber> Times = Data.KeyValues[Data.CurrentIndex].Times;
			TArray<FMovieSceneDoubleValue> Values = Data.KeyValues[Data.CurrentIndex].DoubleValues;
			if (DoubleChannel != nullptr)
			{
				DoubleChannel->Set(Times, Values);
			}
		}
		else if (Data.ChannelTypeName == "MovieSceneIntegerChannel")
		{
			TArray<FFrameNumber> Times = Data.KeyValues[Data.CurrentIndex].Times;
			TArray<int32> Values = Data.KeyValues[Data.CurrentIndex].IntegerValues;
			if (IntegerChannel != nullptr)
			{
				IntegerChannel->Set(Times, Values);
			}
		}
	}
}
bool MotionHandler::operator==(MotionHandler& handler)
{
	return (Data.GetName() == handler.Data.GetName() && Data.ControlSelection == handler.Data.ControlSelection);
}
void MotionHandler::OnScaleValueChangedRaw(double value)
{
	Data.Scale = value;
}
void MotionHandler::OnCurrentIndexValueChangedRaw(int32 value)
{
	Data.CurrentIndex = value;
}
void MotionHandler::OnTextChangedRaw(const FText& value)
{
	Data.CustomName = value;
}
