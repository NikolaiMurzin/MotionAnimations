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
#include "Sequencer/MovieSceneControlRigParameterTrack.h"
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
	enum Mode Mode;

	void SetKey(FFrameNumber InTime, FVector2D InputVector);
	MotionHandler(const IKeyArea* KeyAreas, double DefaultScale_, ISequencer* Sequencer_, UMovieScene* MovieScene_,
		UMovieSceneTrack* MovieSceneTrack_, FGuid ObjectFGuid_, enum Mode Mode_ = Mode::X);
	MotionHandler();
	void InitKeys();

	void DeleteAllKeysFrom(FFrameNumber InTime);
	void DeleteKeysWithin(TRange<FFrameNumber> InRange);

	double GetValueFromTime(FFrameNumber InTime);

	const IKeyArea* KeyArea;

	void Optimize(TRange<FFrameNumber> InRange);

	ISequencer* Sequencer;
	UMovieScene* MovieScene;
	FGuid ObjectFGuid;
	UMovieSceneTrack* MovieSceneTrack;
	UMovieSceneControlRigParameterTrack* MovieSceneControlRigParameterTrack;
	FName currentControlSelection;

private:
	void UpdateUI(FFrameNumber InTime);

	void SyncControlRigWithChannelValue(FFrameNumber InTime);

	int32 channelIndex;
	FMovieSceneFloatChannel* FloatChannel;
	FMovieSceneDoubleChannel* DoubleChannel;
	FMovieSceneBoolChannel* BoolChannel;
	FMovieSceneIntegerChannel* IntegerChannel;
};
