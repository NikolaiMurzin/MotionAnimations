// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

#include "MotionHandler.h"

#include "Channels/MovieSceneBoolChannel.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"
#include "ControlRig.h"
#include "CoreFwd.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "EulerTransform.h"
#include "ISequencer.h"
#include "ISequencerModule.h"
#include "Internationalization/Text.h"
#include "KeyValues.h"
#include "Math/Color.h"
#include "Math/NumericLimits.h"
#include "Math/TransformNonVectorized.h"
#include "MotionHandlerData.h"
#include "MotionHandlerMode.h"
#include "MovieScene/MovieSceneNiagaraTrack.h"
#include "MovieScene/Parameters/MovieSceneNiagaraFloatParameterTrack.h"
#include "MovieScene/Parameters/MovieSceneNiagaraParameterTrack.h"
#include "MovieScene/Parameters/MovieSceneNiagaraVectorParameterTrack.h"
#include "MovieSceneSequence.h"
#include "MovieSceneTrack.h"
#include "MotionEditor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraTypes.h"
#include "RigVMCore/RigVM.h"
#include "RigVMCore/RigVMExternalVariable.h"
#include "Runtime/Core/Public/CoreFwd.h"
#include "Runtime/MovieScene/Public/Channels/MovieSceneChannelHandle.h"
#include "Runtime/MovieScene/Public/MovieSceneSection.h"
#include "Sequencer/MovieSceneControlRigParameterTrack.h"
#include "Sequencer/MovieSceneControlRigSpaceChannel.h"
#include "SequencerAddKeyOperation.h"
#include "SequencerKeyParams.h"
#include "Templates/SharedPointerInternals.h"
#include "Tracks/MovieSceneMaterialTrack.h"
#include "Tracks/MovieSceneParticleParameterTrack.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "Units/RigUnitContext.h"
#include "Accelerator.h"
#include "Math/Range.h"


#include <stdexcept>
#include <string>

MotionHandler::MotionHandler(ISequencer* Sequencer_, UMovieSceneSequence* Sequence, FString FilePath)
{
	Data = FMotionHandlerData(FilePath);
	Sequencer = Sequencer_;

	OnScaleValueChanged.BindRaw(this, &MotionHandler::OnScaleValueChangedRaw);
	OnTextChanged.BindRaw(this, &MotionHandler::OnTextChangedRaw);
	OnCurrentIndexValueChanged.BindRaw(this, &MotionHandler::OnCurrentIndexValueChangedRaw);
	OnLowerBoundValueChanged.BindRaw(this, &MotionHandler::OnLowerBoundValueChangedRaw);
	OnUpperBoundValueChanged.BindRaw(this, &MotionHandler::OnUpperBoundValueChangedRaw);
	OnSetIndexChanged.BindRaw(this, &MotionHandler::OnSetIndexChangedRaw);

	PreviousValue = 0;
	IsFirstUpdate = true;

	MovieSceneControlRigParameterTrack = nullptr;
	MovieSceneMaterialTrack = nullptr;
	MovieSceneNiagaraParameterTrack = nullptr;

	NiagaraComponent = nullptr;

	MAccelerator = nullptr;

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
	MovieSceneSection= nullptr;
	if (MovieSceneTrack == nullptr)
	{
		return;
	}
	// init MovieSceneSection
	for (UMovieSceneSection* Section_ : MovieSceneTrack->GetAllSections())
	{
		if (Section_->GetRowIndex() == Data.SectionRowIndex)
		{
			MovieSceneSection = Section_;
			break;
		}
	}
	if (MovieSceneSection == nullptr)
	{
		return;
	}
	FMovieSceneChannelProxy& channelProxy = MovieSceneSection->GetChannelProxy();
	ChannelHandle = channelProxy.MakeHandle(FName(Data.ChannelTypeName), Data.ChannelIndex);
	if (ChannelHandle.Get() == nullptr)
	{
		return;
	}

	SetControlRigTrack(MovieSceneTrack);
	SetMaterialTrack(MovieSceneTrack);
	SetNiagaraTrack(MovieSceneTrack);
	CastChannel();
	MAccelerator = TSharedPtr<Accelerator>(new Accelerator(FloatChannel, DoubleChannel, IntegerChannel));
	MMotionEditor = TSharedPtr<MotionEditor>(new MotionEditor(FloatChannel, DoubleChannel, IntegerChannel));
}
MotionHandler::MotionHandler(const IKeyArea* KeyArea_, double Scale, ISequencer* Sequencer_,
	UMovieSceneTrack* MovieSceneTrack_, FGuid ObjectFGuid_, Mode Mode_)
{
	Sequencer = Sequencer_;
	KeyArea = KeyArea_;
	PreviousValue = 0;
	IsFirstUpdate = true;
	MovieScene = MovieSceneTrack_->GetTypedOuter<UMovieScene>();
	MovieSceneTrack = MovieSceneTrack_;

	MovieSceneSection = KeyArea->GetOwningSection();

	MovieSceneControlRigParameterTrack = nullptr;
	MovieSceneMaterialTrack = nullptr;
	MovieSceneNiagaraParameterTrack = nullptr;

	NiagaraComponent = nullptr;

	MAccelerator = nullptr;

	OnScaleValueChanged.BindRaw(this, &MotionHandler::OnScaleValueChangedRaw);
	OnTextChanged.BindRaw(this, &MotionHandler::OnTextChangedRaw);
	OnCurrentIndexValueChanged.BindRaw(this, &MotionHandler::OnCurrentIndexValueChangedRaw);
	OnLowerBoundValueChanged.BindRaw(this, &MotionHandler::OnLowerBoundValueChangedRaw);
	OnUpperBoundValueChanged.BindRaw(this, &MotionHandler::OnUpperBoundValueChangedRaw);
	OnSetIndexChanged.BindRaw(this, &MotionHandler::OnSetIndexChangedRaw);

	FString TrackName_ = MovieSceneTrack_->GetTrackName().ToString();
	int32 RowIndex = KeyArea->GetOwningSection()->GetRowIndex();

	FString ChannelTypeName = KeyArea->GetChannel().GetChannelTypeName().ToString();
	int32 ChannelIndex = KeyArea->GetChannel().GetChannelIndex();

	ChannelHandle = KeyArea->GetChannel();
	const FMovieSceneChannelMetaData* metadata = ChannelHandle.GetMetaData();
	FText customName = FText::FromString("");
	if (metadata) {
		customName = ChannelHandle.GetMetaData()->DisplayText;
	}
	FString MovieSceneTrackName = MovieSceneTrack_->GetDisplayName().ToString();

	FName keyAreaName = KeyArea->GetName();
	FText DataCustomName = FText::FromString(keyAreaName.ToString());
	UMovieSceneSequence* Sequence = nullptr;

	if (MovieScene)
	{
		// Get the outer sequence object
		UObject* OuterObject = MovieScene->GetOuter();

		while (OuterObject)
		{
			// Check if the outer object is a UMovieSceneSequence
			Sequence = Cast<UMovieSceneSequence>(OuterObject);
			
			if (Sequence)
			{
				// Found the sequence object
				break;
			}
			
			// Traverse up the outer object hierarchy
			OuterObject = OuterObject->GetOuter();
		}

		if (!Sequence)
		{
			return;
		}
		else
		{
			// Now you have access to the UMovieSceneSequence object
			// You can use this Sequence pointer as needed
		}
	}

	Data = FMotionHandlerData(Scale, ObjectFGuid_, TrackName_, RowIndex, ChannelTypeName, ChannelIndex, Mode_,
		Sequence->GetDisplayName().ToString(), DataCustomName, customName,
		KeyArea->GetName().ToString(), MovieSceneTrackName);
	SetControlRigTrack(MovieSceneTrack);
	SetMaterialTrack(MovieSceneTrack);
	SetNiagaraTrack(MovieSceneTrack);
	CastChannel();
	MAccelerator = TSharedPtr<Accelerator>(new Accelerator(FloatChannel, DoubleChannel, IntegerChannel));
	MMotionEditor = TSharedPtr<MotionEditor>(new MotionEditor(FloatChannel, DoubleChannel, IntegerChannel));
}

bool MotionHandler::IsValidMotionHandler()
{
	if (Sequencer == nullptr || MovieScene == nullptr || MovieSceneTrack == nullptr)
	{
		ValidMotionHandler = false;
		return false;
	}
	bool ChannelNullPtr =
		(FloatChannel == nullptr && DoubleChannel == nullptr && IntegerChannel == nullptr);
	bool IsValid_ = !ChannelNullPtr;
	if (!IsValid_)
	{
		UE_LOG(LogTemp, Warning, TEXT("Motion handler is not valid"));
	}
	ValidMotionHandler = IsValid_;
	return IsValid_;
}

void MotionHandler::SetControlRigTrack(UMovieSceneTrack* MovieSceneTrack_)
{
	MovieSceneControlRigParameterTrack = Cast<UMovieSceneControlRigParameterTrack>(MovieSceneTrack_);

	if (IsValid(MovieSceneControlRigParameterTrack))
	{
		controlRig = MovieSceneControlRigParameterTrack->GetControlRig();
		TArray<FName> currentControlSelectionArr = MovieSceneControlRigParameterTrack->GetControlRig()->CurrentControlSelection();
		if (currentControlSelectionArr.Num() > 0)
		{
			TArray<FString> out;
			FString controlName;
			FString keyAreaName = Data.KeyAreaName;
			keyAreaName.ParseIntoArray(out, TEXT("."), true);
			if (out.Num() > 0)
			{
				controlName = out[0];
			}
			for (FName controlSelectionName : currentControlSelectionArr)
			{
				if (controlName == controlSelectionName.ToString())
				{
					Data.ControlSelection = controlSelectionName.ToString();
				}
			}
		}
	}
}
void MotionHandler::SetMaterialTrack(UMovieSceneTrack* MovieSceneTrack_)
{
	UMovieSceneMaterialTrack* MovieSceneMaterialTrack_ = Cast<UMovieSceneMaterialTrack>(MovieSceneTrack_);
	if (MovieSceneMaterialTrack_ != nullptr && IsValid(MovieSceneMaterialTrack_))
	{
		MovieSceneMaterialTrack = MovieSceneMaterialTrack_;
	}
}
void MotionHandler::SetNiagaraTrack(UMovieSceneTrack* MovieSceneTrack_)
{
	UMovieSceneParticleParameterTrack* MovieSceneParticleTrack_ = Cast<UMovieSceneParticleParameterTrack>(MovieSceneTrack_);
	UMovieSceneNiagaraTrack* MovieSceneNiagaraTrack_ = Cast<UMovieSceneNiagaraTrack>(MovieSceneTrack_);
	UMovieSceneNiagaraParameterTrack* MovieSceneNiagaraParameterTrack_ = Cast<UMovieSceneNiagaraParameterTrack>(MovieSceneTrack_);
	if (IsValid(MovieSceneNiagaraTrack_))
	{
		MovieSceneNiagaraTrack = MovieSceneNiagaraTrack_;

		UObject* Obj = Sequencer->FindSpawnedObjectOrTemplate(Data.ObjectFGuid);
		if (Obj != nullptr)
		{
			UNiagaraComponent* NiagaraComponent_ = Cast<UNiagaraComponent>(Obj);
			if (IsValid(NiagaraComponent_))
			{
				NiagaraComponent = NiagaraComponent_;
			}
		}

		// for determine which type of param we have
		if (IsValid(MovieSceneNiagaraParameterTrack_))
		{
			MovieSceneNiagaraParameterTrack = MovieSceneNiagaraParameterTrack_;
			UMovieSceneNiagaraFloatParameterTrack* MovieSceneNiagaraFloatTrack_ =
				Cast<UMovieSceneNiagaraFloatParameterTrack>(MovieSceneNiagaraParameterTrack_);
			if (MovieSceneNiagaraFloatTrack_ != nullptr)
			{
				MovieSceneNiagaraFloatTrack = MovieSceneNiagaraFloatTrack_;
			}
			UMovieSceneNiagaraVectorParameterTrack* MovieSceneNiagaraVectorTrack_ =
				Cast<UMovieSceneNiagaraVectorParameterTrack>(MovieSceneNiagaraParameterTrack_);
			if (MovieSceneNiagaraVectorTrack_ != nullptr)
			{
				MovieSceneNiagaraVectorTrack = MovieSceneNiagaraVectorTrack_;
			}
		}
	}
}
void MotionHandler::SyncNiagaraParam(FFrameNumber InTime, float value)
{
	if (MovieSceneNiagaraParameterTrack == nullptr)
	{
		return;
	}
	if (IsValid(MovieSceneNiagaraParameterTrack))
	{
		if (FloatChannel != nullptr)
		{
			if (NiagaraComponent != nullptr)
			{
				FNiagaraVariable var = MovieSceneNiagaraParameterTrack->GetParameter();
				if (MovieSceneNiagaraFloatTrack != nullptr)
				{
					NiagaraComponent->SetFloatParameter(var.GetName(), value);
				}
				else if (MovieSceneNiagaraVectorTrack != nullptr)
				{
					FVector vec = FVector();
					FString VectorString = NiagaraComponent->GetOverrideParameters().ToString();

					// Find the starting position of the X component
					int32 StartIndex = VectorString.Find("X:") + 2;

					// Find the ending position of the X component
					int32 EndIndex = VectorString.Find("Y:") - 1;

					// Extract the X component from the string
					FString XString = VectorString.Mid(StartIndex, EndIndex - StartIndex);
					float XValue = FCString::Atof(*XString);

					// Find the starting position of the Y component
					StartIndex = VectorString.Find("Y:") + 2;

					// Find the ending position of the Y component
					EndIndex = VectorString.Find("Z:") - 1;

					// Extract the Y component from the string
					FString YString = VectorString.Mid(StartIndex, EndIndex - StartIndex);
					float YValue = FCString::Atof(*YString);

					// Find the starting position of the Z component
					StartIndex = VectorString.Find("Z:") + 2;

					// Find the ending position of the Z component
					EndIndex = VectorString.Len() - 2;

					// Extract the Z component from the string
					FString ZString = VectorString.Mid(StartIndex, EndIndex - StartIndex);
					float ZValue = FCString::Atof(*ZString);
					FString axis = *Data.ChannelDisplayText.ToString();
					if (axis == "X")
					{
						vec.X = value;
						vec.Y = YValue;
						vec.Z = ZValue;
					}
					else if (axis == "Y")
					{
						vec.X = XValue;
						vec.Y = value;
						vec.Z = ZValue;
					}
					else if (axis == "Z")
					{
						vec.X = XValue;
						vec.Y = YValue;
						vec.Z = value;
					}
					NiagaraComponent->SetVectorParameter(var.GetName(), vec);
				}
			}
		}
	}
}
void MotionHandler::ResetNiagaraState()
{
	if (IsValid(NiagaraComponent))
	{
		NiagaraComponent->DeactivateImmediate();
		NiagaraComponent->ActivateSystem();
	}
}
void MotionHandler::SyncMaterialTrack(FFrameNumber InTime, float value)
{
	if (MovieSceneMaterialTrack == nullptr)
	{
		return;
	}
	if (IsValid(MovieSceneMaterialTrack))
	{
		FName ParameterName = FName(*Data.ChannelDisplayText.ToString());
		FString ChannelDisplayText = Data.ChannelDisplayText.ToString();
		if (FloatChannel == nullptr)
		{
			return;
		}
		if (ChannelDisplayText.Equals("R") || ChannelDisplayText.Equals("G") || ChannelDisplayText.Equals("B") ||
			ChannelDisplayText.Equals("A"))
		{
			FString ParamName = Data.KeyAreaName;
			ParamName = ParamName.LeftChop(1);

			FLinearColor color = FLinearColor();
			if (ChannelDisplayText.Equals("R"))
			{
				color.R = value;
				color.G = 0;
				color.B = 0;
				color.A = 0;
			}
			else if (ChannelDisplayText.Equals("G"))
			{
				color.R = 0;
				color.G = value;
				color.B = 0;
				color.A = 0;
			}
			else if (ChannelDisplayText.Equals("B"))
			{
				color.R = 0;
				color.G = 0;
				color.B = value;
				color.A = 0;
			}
			else if (ChannelDisplayText.Equals("A"))
			{
				color.R = 0;
				color.G = 0;
				color.B = 0;
				color.A = value;
			}
			MovieSceneMaterialTrack->AddColorParameterKey(FName(*ParamName), InTime, color);
		}
		else
		{
			MovieSceneMaterialTrack->AddScalarParameterKey(ParameterName, InTime, value);
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

	double Scale = Data.Scale / 100;

	valueToSet = valueToSet * Scale + PreviousValue;

	if (Data.UpperBoundValue != Data.LowerBoundValue)	 // if it's equal then user don't won't to use bounds
	{
		if (valueToSet >= Data.UpperBoundValue)
		{
			valueToSet = Data.UpperBoundValue;
		}
		else if (valueToSet <= Data.LowerBoundValue)
		{
			valueToSet = Data.LowerBoundValue;
		}
	}


	if (Data.ChannelTypeName == "MovieSceneFloatChannel")
	{
		valueToSet = (float)valueToSet;
		if (!FloatChannel->HasAnyData())
		{
			UE_LOG(LogTemp, Warning, TEXT("Can't get data of float channel"));
			return;
		}
		TMovieSceneChannelData<FMovieSceneFloatValue>FloatChannelData = FloatChannel->GetData();

		FloatChannelData.UpdateOrAddKey(InTime, FMovieSceneFloatValue(valueToSet));
	}
	else if (Data.ChannelTypeName == "MovieSceneDoubleChannel")
	{
		valueToSet = (double)valueToSet;
		if (!DoubleChannel->HasAnyData())
		{
			UE_LOG(LogTemp, Warning, TEXT("Can't get data of double channel"));
			return;
		}

		TMovieSceneChannelData<FMovieSceneDoubleValue>DoubleChannelData = DoubleChannel->GetData();

		DoubleChannelData.UpdateOrAddKey(InTime, FMovieSceneDoubleValue(valueToSet));

	}
	else if (Data.ChannelTypeName == "MovieSceneIntegerChannel")
	{
		valueToSet = (int32)valueToSet;
		if (!IntegerChannel->HasAnyData())
		{
			UE_LOG(LogTemp, Warning, TEXT("Can't get data of float channel"));
			return;
		}
		TMovieSceneChannelData<int>IntegerChannelData = IntegerChannel->GetData();

		IntegerChannelData.UpdateOrAddKey(InTime, valueToSet);
	}
	SyncMaterialTrack(InTime, valueToSet);
	SyncControlRigWithChannelValue(InTime, valueToSet);
	SyncNiagaraParam(InTime, valueToSet);

	PreviousValue = valueToSet;
}
void MotionHandler::EditPosition(FFrameNumber InTime, FVector2D InputVector)
{
	if (!ValidMotionHandler)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't set key, motion handler is not valid"));
		return;
	}
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
	double Scale = Data.Scale / 100;
	valueToSet *= Scale;

	MMotionEditor->Edit(InTime, valueToSet);

	SyncMaterialTrack(InTime, valueToSet);
	SyncControlRigWithChannelValue(InTime, valueToSet);
	SyncNiagaraParam(InTime, valueToSet);
}
void MotionHandler::ResetMotionEditor(TRange<FFrameNumber> range)
{

	// MMotionEditor->Reset(range);
}
void MotionHandler::ReInitMotionEditor()
{
	// MMotionEditor->ReInit();
}
void MotionHandler::SyncControlRigWithChannelValue(FFrameNumber InTime, float valueOfChannel)
{
	if (IsValid(MovieSceneControlRigParameterTrack))
	{
		if (controlElement == nullptr)
		{
			controlElement = controlRig->FindControl(FName(Data.ControlSelection));
			if (controlElement != nullptr)
			{
				controlType = controlElement->Settings.ControlType;
				controlValueMin = controlElement->Settings.MinimumValue;
				controlValueMax = controlElement->Settings.MaximumValue;
			}
			else
			{
				UE_LOG(LogTemp, Warning,
					TEXT("motion handler is not valid, maybe you forget to select "
						"control in sequencer when tried to init motion "
						"handler?"));
				return;
			}
		}

		if (controlElement == nullptr)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("motion handler is not valid, maybe you forget to select "
					"control in sequencer when tried to init motion "
					"handler?"));
			return;
		}
		if (FloatChannel == nullptr)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("motion handler is not valid, it's float channel is null"));
			return;
		}

		controlValue = controlRig->GetControlValue(FName(Data.ControlSelection));
		FString ChannelDisplayText = Data.ChannelDisplayText.ToString();

		if (controlType == ERigControlType::Float)
		{
			controlRig->SetControlValue(
				FName(Data.ControlSelection), valueOfChannel, true, FRigControlModifiedContext(), true, true);
		}
		else if (controlType == ERigControlType::EulerTransform)
		{
			FEulerTransform eulerValue;
			eulerValue.FromFTransform(controlValue.GetAsTransform(ERigControlType::EulerTransform, ERigControlAxis::X));

			/* ue log max and min values todo */
			if (ChannelDisplayText == "Location.X")
			{
				FVector location = eulerValue.GetLocation();
				location.X = valueOfChannel;
				eulerValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Location.Y")
			{
				FVector location = eulerValue.GetLocation();
				location.Y = valueOfChannel;
				eulerValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Location.Z")
			{
				FVector location = eulerValue.GetLocation();
				location.Z = valueOfChannel;
				eulerValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Rotation.Roll")
			{
				UE::Math::TQuat<double> rotation = eulerValue.GetRotation();
				FRotator rotator = rotation.Rotator();
				rotator.Roll = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				eulerValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Rotation.Pitch")
			{
				UE::Math::TQuat<double> rotation = eulerValue.GetRotation();
				FRotator rotator = rotation.Rotator();
				rotator.Pitch = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				eulerValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Rotation.Yaw")
			{
				UE::Math::TQuat<double> rotation = eulerValue.GetRotation();
				FRotator rotator = rotation.Rotator();
				rotator.Yaw = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				eulerValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Scale.X")
			{
				FVector scale = eulerValue.GetScale3D();
				scale.X = valueOfChannel;
				eulerValue.SetScale3D(scale);
			}
			else if (ChannelDisplayText == "Scale.Y")
			{
				FVector scale = eulerValue.GetScale3D();
				scale.Y = valueOfChannel;
				eulerValue.SetScale3D(scale);
			}
			else if (ChannelDisplayText == "Scale.Z")
			{
				FVector scale = eulerValue.GetScale3D();
				scale.Z = valueOfChannel;
				eulerValue.SetScale3D(scale);
			}
			controlRig->SetControlValue(FName(Data.ControlSelection), eulerValue, true, FRigControlModifiedContext(), true, true);
		}
		else if (controlType == ERigControlType::TransformNoScale)
		{
			FTransformNoScale nValue = controlValue.GetAsTransform(ERigControlType::TransformNoScale, ERigControlAxis::X);
			if (ChannelDisplayText == "Location.X")
			{
				FVector location = nValue.Location;
				location.X = valueOfChannel;
				nValue.Location = location;
			}
			else if (ChannelDisplayText == "Location.Y")
			{
				FVector location = nValue.Location;
				location.Y = valueOfChannel;
				nValue.Location = location;
			}
			else if (ChannelDisplayText == "Location.Z")
			{
				FVector location = nValue.Location;
				location.Z = valueOfChannel;
				nValue.Location = location;
			}
			else if (ChannelDisplayText == "Rotation.Roll")
			{
				UE::Math::TQuat<double> rotation = nValue.Rotation;
				FRotator rotator = rotation.Rotator();
				rotator.Roll = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				nValue.Rotation = rotation;
			}
			else if (ChannelDisplayText == "Rotation.Pitch")
			{
				UE::Math::TQuat<double> rotation = nValue.Rotation;
				FRotator rotator = rotation.Rotator();
				rotator.Pitch = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				nValue.Rotation = rotation;
			}
			else if (ChannelDisplayText == "Rotation.Yaw")
			{
				UE::Math::TQuat<double> rotation = nValue.Rotation;
				FRotator rotator = rotation.Rotator();
				rotator.Yaw = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				nValue.Rotation = rotation;
			}
		}
		else if (controlType == ERigControlType::Transform)
		{
			FTransform eulerValue = controlValue.GetAsTransform(ERigControlType::Transform, ERigControlAxis::X);
			if (ChannelDisplayText == "Location.X")
			{
				FVector location = eulerValue.GetLocation();
				location.X = valueOfChannel;
				eulerValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Location.Y")
			{
				FVector location = eulerValue.GetLocation();
				location.Y = valueOfChannel;
				eulerValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Location.Z")
			{
				FVector location = eulerValue.GetLocation();
				location.Z = valueOfChannel;
				eulerValue.SetLocation(location);
			}
			else if (ChannelDisplayText == "Rotation.Roll")
			{
				UE::Math::TQuat<double> rotation = eulerValue.GetRotation();
				FRotator rotator = rotation.Rotator();
				rotator.Roll = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				eulerValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Rotation.Pitch")
			{
				UE::Math::TQuat<double> rotation = eulerValue.GetRotation();
				FRotator rotator = rotation.Rotator();
				rotator.Pitch = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				eulerValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Rotation.Yaw")
			{
				UE::Math::TQuat<double> rotation = eulerValue.GetRotation();
				FRotator rotator = rotation.Rotator();
				rotator.Yaw = valueOfChannel;
				rotation = UE::Math::TQuat<double>::MakeFromRotator(rotator);
				eulerValue.SetRotation(rotation);
			}
			else if (ChannelDisplayText == "Scale.X")
			{
				FVector scale = eulerValue.GetScale3D();
				scale.X = valueOfChannel;
				eulerValue.SetScale3D(scale);
			}
			else if (ChannelDisplayText == "Scale.Y")
			{
				FVector scale = eulerValue.GetScale3D();
				scale.Y = valueOfChannel;
				eulerValue.SetScale3D(scale);
			}
			else if (ChannelDisplayText == "Scale.Z")
			{
				FVector scale = eulerValue.GetScale3D();
				scale.Z = valueOfChannel;
				eulerValue.SetScale3D(scale);
			}
			controlRig->SetControlValue(FName(Data.ControlSelection), eulerValue, true, FRigControlModifiedContext(), false, false);
		}
		else if (controlType == ERigControlType::Vector2D)
		{
			FString eulerValue = controlValue.ToString<FVector2D>();
			FVector currentValue = GetVectorFromString(*eulerValue);

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
double MotionHandler::GetValueFromTime(FFrameNumber InTime)
{
	if (!IsValidMotionHandler())
	{
		return double(0);
	}
	if (Data.ChannelTypeName == "MovieSceneFloatChannel")
	{
		if (FloatChannel != nullptr && FloatChannel->HasAnyData())
		{
			float result = 0;
			FloatChannel->Evaluate(InTime, result);
			return result;
		}
	}
	if (Data.ChannelTypeName == "MovieSceneDoubleChannel")
	{
		if (DoubleChannel != nullptr && DoubleChannel->HasAnyData())
		{
			double result = 0;
			DoubleChannel->Evaluate(InTime, result);
			return result;
		}
	}
	if (Data.ChannelTypeName == "MovieSceneIntegerChannel")
	{
		if (IntegerChannel != nullptr && IntegerChannel->HasAnyData())
		{
			int32 result = 0;
			IntegerChannel->Evaluate(InTime, result);
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
	TArray<FKeyHandle> KeyHandles = TArray<FKeyHandle>();
	if (Data.ChannelTypeName == "MovieSceneFloatChannel")
	{
		if (FloatChannel != nullptr)
		{
			FloatChannel->GetKeys(InRange, nullptr, &KeyHandles);
			FloatChannel->DeleteKeys(TArrayView<const FKeyHandle>(KeyHandles));
		}
	}
	else if (Data.ChannelTypeName == "MovieSceneDoubleChannel")
	{
		if (DoubleChannel != nullptr)
		{
			DoubleChannel->GetKeys(InRange, nullptr, &KeyHandles);
			DoubleChannel->DeleteKeys(TArrayView<const FKeyHandle>(KeyHandles));
		}
	}
	else if (Data.ChannelTypeName == "MovieSceneIntegerChannel")
	{
		if (IntegerChannel != nullptr)
		{
			IntegerChannel->GetKeys(InRange, nullptr, &KeyHandles);
			IntegerChannel->DeleteKeys(TArrayView<const FKeyHandle>(KeyHandles));
		}
	}
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
void MotionHandler::Optimize(TRange<FFrameNumber> InRange, double tolerance)
{
	if (!IsValidMotionHandler())
	{
		return;
	}
	FKeyDataOptimizationParams params = FKeyDataOptimizationParams();
	params.bAutoSetInterpolation = true;
	params.Range = InRange;
	params.Tolerance = tolerance;
	params.DisplayRate = FFrameRate();
	params.DisplayRate.Numerator = 24;
	params.DisplayRate.Denominator = 1;

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
		Data.KeyValues.Add(Data.CurrentIndex, KeyValues);
	}
	else if (Data.ChannelTypeName == "MovieSceneDoubleChannel")
	{
		FKeyValues KeyValues = FKeyValues();
		KeyValues.DoubleValues = DoubleChannel->GetValues();
		KeyValues.Times = DoubleChannel->GetTimes();
		Data.KeyValues.Add(Data.CurrentIndex, KeyValues);
	}
	else if (Data.ChannelTypeName == "MovieSceneIntegerChannel")
	{
		FKeyValues KeyValues = FKeyValues();
		KeyValues.IntegerValues = IntegerChannel->GetValues();
		KeyValues.Times = IntegerChannel->GetTimes();
		Data.KeyValues.Add(Data.CurrentIndex, KeyValues);
	}
}
void MotionHandler::AddOrUpdateKeyValueInSequencer()
{
	if (!IsValidMotionHandler())
	{
		return;
	}
	if (Data.KeyValues.Contains(Data.CurrentIndex))
	{
		UpdateCurrentKeyValues();
	}
	else
	{
		AddNewKeyValues();
	}
}
void MotionHandler::UpdateCurrentKeyValues()
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
	Data.KeyValues.FindAndRemoveChecked(Data.CurrentIndex);
}
void MotionHandler::InsertCurrentKeyValuesToSequencer()
{
	if (!IsValidMotionHandler())
	{
		return;
	}
	if (MovieScene != nullptr)
	{
		if (!Data.KeyValues.Contains(Data.CurrentIndex))
		{
			return;
		}
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

void MotionHandler::Accelerate(FVector2D value, FFrameNumber keyTime)
{
	if (!ValidMotionHandler)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't set key, motion handler is not valid"));
		return;
	}
	double valueToSet = 0;
	valueToSet = value.X * -1; // take from x by default

	double val = valueToSet;

	double Scale = Data.Scale / 100;
	valueToSet = valueToSet * Scale + AccelerateLastValue * 0.01;

	MAccelerator->Accelerate(valueToSet, keyTime);

	SyncMaterialTrack(keyTime, valueToSet);
	SyncControlRigWithChannelValue(keyTime, valueToSet);
	SyncNiagaraParam(keyTime, valueToSet);

	AccelerateLastValue = val;
}
void MotionHandler::ResetAccelerator(TRange<FFrameNumber> range)
{
	FFrameNumber from = range.GetLowerBoundValue();
	from.Value += 1000;
	range.SetLowerBound(from);
	MAccelerator->Reset(range);
}
void MotionHandler::ReInitAccelerator(TRange<FFrameNumber> range)
{
	if (MAccelerator != nullptr)
	{
		MAccelerator->UpdateBackup(range);
	}
}
void MotionHandler::Populate(TRange<FFrameNumber> range, FFrameNumber interval)
{
	if (FloatChannel != nullptr)
	{
		FFrameNumber curr = range.GetLowerBoundValue();
		while (curr < range.GetUpperBoundValue())
		{
			float value;
			FloatChannel->Evaluate(curr, value);
			FloatChannel->GetData().UpdateOrAddKey(curr, FMovieSceneFloatValue(value));
			curr += interval;
		}
		FloatChannel->AutoSetTangents();
	}
	else if (DoubleChannel != nullptr)
	{
		// go with some intevral and evaluate and paste new keys at times with given interval
		FFrameNumber curr = range.GetLowerBoundValue();
		while (curr < range.GetUpperBoundValue())
		{
			double value;
			DoubleChannel->Evaluate(curr, value);
			DoubleChannel->GetData().UpdateOrAddKey(curr, FMovieSceneDoubleValue(value));
			curr += interval;
		}
		DoubleChannel->AutoSetTangents();
	}
	else if (IntegerChannel != nullptr)
	{
		FFrameNumber curr = range.GetLowerBoundValue();
		while (curr < range.GetUpperBoundValue())
		{
			int value;
			IntegerChannel->Evaluate(curr, value);
			IntegerChannel->GetData().UpdateOrAddKey(curr, value);
			curr += interval;
		}
		FloatChannel->AutoSetTangents();
	}
}
bool MotionHandler::operator==(MotionHandler& handler)
{
	return (Data.GetName() == handler.Data.GetName() && Data.ControlSelection == handler.Data.ControlSelection &&
		Data.ChannelDisplayText.ToString() == handler.Data.ChannelDisplayText.ToString() &&
		Data.KeyAreaName == handler.Data.KeyAreaName && Data.TrackDisplayName == handler.Data.TrackDisplayName && handler.Data.ObjectFGuid == Data.ObjectFGuid);
}
bool MotionHandler::HasData()
{
	if (FloatChannel != nullptr)
	{
		return FloatChannel->GetNumKeys() >= 1 || !Data.KeyValues.IsEmpty();
	}
	else if (DoubleChannel != nullptr)
	{
		return DoubleChannel->GetNumKeys() >= 1 || !Data.KeyValues.IsEmpty();
	}
	else if (IntegerChannel != nullptr)
	{
		return IntegerChannel->GetNumKeys() >= 1 || !Data.KeyValues.IsEmpty();
	}
	return false;
}
void MotionHandler::OnScaleValueChangedRaw(double value)
{
	Data.Scale = value;
}
void MotionHandler::OnCurrentIndexValueChangedRaw(int32 value)
{
	Data.CurrentIndex = value;
}
void MotionHandler::OnUpperBoundValueChangedRaw(double value)
{
	Data.UpperBoundValue = value;
}
void MotionHandler::OnLowerBoundValueChangedRaw(double value)
{
	Data.LowerBoundValue = value;
}
void MotionHandler::OnTextChangedRaw(const FText& value)
{
	Data.CustomName = value;
}
void MotionHandler::OnSetIndexChangedRaw(int32 value)
{
	Data.SetIndex = value;
}
FMovieSceneChannelHandle MotionHandler::GetActualChannelHandle()
{
	FMovieSceneChannelProxy& channelProxy = MovieSceneSection->GetChannelProxy();
	return channelProxy.MakeHandle(FName(Data.ChannelTypeName),Data.ChannelIndex);
}
