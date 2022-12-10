#include "MotionHandler.h"

#include "Channels/MovieSceneBoolChannel.h"
#include "Channels/MovieSceneChannel.h"
#include "Channels/MovieSceneChannelHandle.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"
#include "ControlRig.h"
#include "CoreFwd.h"
#include "Editor.h"
#include "ISequencer.h"
#include "ISequencerModule.h"
#include "Internationalization/Text.h"
#include "PluginsLsp/Animation/ControlRig/Source/ControlRig/Public/ControlRig.h"
#include "PluginsLsp/Animation/ControlRig/Source/ControlRig/Public/Sequencer/MovieSceneControlRigParameterTrack.h"
#include "RigVMCore/RigVM.h"
#include "Runtime/RigVM/Public/RigVMCore/RigVM.h"
#include "Sequencer/MovieSceneControlRigParameterTrack.h"
#include "Sequencer/MovieSceneControlRigSpaceChannel.h"
#include "SequencerAddKeyOperation.h"
#include "SequencerKeyParams.h"
#include "SequencerWidgets/Public/ITimeSlider.h"
#include "Templates/SharedPointerInternals.h"
#include "UObject/NameTypes.h"
#include "Units/RigUnitContext.h"

#include <stdexcept>

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
		FloatChannel->PostEditChange();
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
		DoubleChannel->PostEditChange();
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
		IntegerChannel->PostEditChange();
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
	UE_LOG(LogTemp, Warning, TEXT("channel name is %s"), *Channel.GetMetaData()->Name.ToString());
	UE_LOG(LogTemp, Warning, TEXT("channel display text is %s"), *Channel.GetMetaData()->DisplayText.ToString());
	UE_LOG(LogTemp, Warning, TEXT("channel metadata group is %s"), *Channel.GetMetaData()->Group.ToString());

	UMovieSceneControlRigParameterTrack* ControlRigTrack = Cast<UMovieSceneControlRigParameterTrack>(MovieSceneTrack);
	if (IsValid(ControlRigTrack))
	{
		UE_LOG(LogTemp, Warning, TEXT("it's control rig track! %s"), *ControlRigTrack->GetTrackName().ToString());
		UControlRig* controlRig = ControlRigTrack->GetControlRig();

		controlRig->RequestInit();
		controlRig->RequestSetup();
		controlRig->Execute(EControlRigState::Update, FName("Update"));
		controlRig->Evaluate_AnyThread();

		UE_LOG(LogTemp, Warning, TEXT("category is %s"), *controlRig->GetCategory().ToString());
		TArray<FName> currentControlSelection = controlRig->CurrentControlSelection();
		for (FName name : currentControlSelection)
		{
			UE_LOG(LogTemp, Warning, TEXT("fname control rig %s"), *name.ToString());
			FRigControlValue value = controlRig->GetControlValue(name);
			float val = value.Get<float>();
			UE_LOG(LogTemp, Warning, TEXT("value of control rig is %f"), val);
			float valueOfChannel = 0;
			FloatChannel->Evaluate(InTime, valueOfChannel);
			UE_LOG(LogTemp, Warning, TEXT("value of channel is %f"), valueOfChannel);
			value.Set<float>(valueOfChannel);
			controlRig->SetControlValue(name, valueOfChannel, true, FRigControlModifiedContext(), true, true);
			UE_LOG(
				LogTemp, Warning, TEXT("value after change with full path is %f"), controlRig->GetControlValue(name).Get<float>());
			UE_LOG(LogTemp, Warning, TEXT("value after change with short path is %f"), value.Get<float>());
			value.Set(valueOfChannel);
			controlRig->SetControlValue(name, valueOfChannel, true, FRigControlModifiedContext(), true, true);
			UE_LOG(LogTemp, Warning, TEXT("value after change with short path 2nd is %f"), value.Get<float>());
			UE_LOG(LogTemp, Warning, TEXT("value after change with full path 2nd is %f"),
				controlRig->GetControlValue(name).Get<float>());
			controlRig->SetControlValue(name, valueOfChannel, true, FRigControlModifiedContext(), true, true);
			UE_LOG(LogTemp, Warning, TEXT("value after change with short path 3nd is %f"), value.Get<float>());
			UE_LOG(LogTemp, Warning, TEXT("value after change with full path 3nd is %f"),
				controlRig->GetControlValue(name).Get<float>());
		}
		for (FRigVMExternalVariable var : controlRig->GetPublicVariables())
		{
			UE_LOG(LogTemp, Warning, TEXT("public variable name is %s"), *var.Name.ToString());
		}
		controlRig->SetManipulationEnabled(true);
		UE_LOG(
			LogTemp, Warning, TEXT("Control rig manipulation enabled %S"), (controlRig->ManipulationEnabled() ? "true" : "false"));
	}
}
void MotionHandler::InitKeys()
{
	TRange<FFrameNumber> playbackRange = MovieScene->GetPlaybackRange();
	FFrameNumber lowerValue = playbackRange.GetLowerBoundValue();
	FFrameNumber highValue = playbackRange.GetUpperBoundValue();
	/* todo list */
}

void MotionHandler::UpdateUI(FFrameNumber InTime)
{
	TSharedRef<ISequencerTrackEditor> SequencerEditor = FSpawnTrackEditor::CreateTrackEditor(TSharedRef<ISequencer>(Sequencer));

	using namespace UE::Sequencer;

	TArray<TSharedRef<IKeyArea>> ka = TArray<TSharedRef<IKeyArea>>();
	ka.Add(TSharedRef<IKeyArea>(const_cast<IKeyArea*>(KeyArea)));
	TArrayView<TSharedRef<IKeyArea>> ar = TArrayView<TSharedRef<IKeyArea>>(ka);

	FAddKeyOperation::FromKeyAreas(&*SequencerEditor, ar).Commit(InTime, *Sequencer);
}
