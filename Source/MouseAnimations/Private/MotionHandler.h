#pragma once

#include "Channels/MovieSceneBoolChannel.h"
#include "Channels/MovieSceneChannel.h"
#include "Channels/MovieSceneChannelHandle.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"
#include "CoreFwd.h"
#include "Editor/MovieSceneTools/Public/TrackEditors/SpawnTrackEditor.h"
#include "Editor/Sequencer/Public/IKeyArea.h"
#include "Editor/Sequencer/Public/ISequencerTrackEditor.h"
#include "Editor/Sequencer/Public/SequencerAddKeyOperation.h"
#include "Misc/FrameNumber.h"
#include "Sequencer/Public/ISequencer.h"
#include "UObject/NameTypes.h"

enum Mode : uint8
{
	X,
	XInverted,
	Y,
	YInverted,
};
class MotionHandler
{
public:
	double Scale;
	double PreviousValue;
	bool IsFirstUpdate;
	FName ChannelTypeName;
	void SetKey(FFrameNumber InTime, FVector2D InputVector, Mode Mode);
	MotionHandler(const IKeyArea* KeyAreas, double DefaultScale_, ISequencer* Sequencer_, UMovieScene* MovieScene_,
		UMovieSceneTrack* MovieSceneTrack_, FGuid ObjectFGuid_);
	void InitKeys();

	const IKeyArea* KeyArea;

	ISequencer* Sequencer;
	UMovieScene* MovieScene;
	FGuid ObjectFGuid;
	UMovieSceneTrack* MovieSceneTrack;

private:
	void UpdateUI(FFrameNumber InTime);

	FMovieSceneFloatChannel* FloatChannel;
	FMovieSceneDoubleChannel* DoubleChannel;
	FMovieSceneBoolChannel* BoolChannel;
	FMovieSceneIntegerChannel* IntegerChannel;
};