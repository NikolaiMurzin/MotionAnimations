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
#include "MotionHandlerData.h"
#include "MotionHandlerMode.h"
#include "Sequencer/MovieSceneControlRigParameterTrack.h"
#include "Sequencer/Public/ISequencer.h"
#include "UObject/NameTypes.h"

class MotionHandler
{
public:
	double PreviousValue;
	bool IsFirstUpdate;

	void SetKey(FFrameNumber InTime, FVector2D InputVector);
	MotionHandler(const IKeyArea* KeyAreas, double DefaultScale_, ISequencer* Sequencer_, UMovieSceneSequence* Sequence_,
		UMovieSceneTrack* MovieSceneTrack_, FGuid ObjectFGuid_, Mode Mode_ = Mode::X);
	MotionHandler(ISequencer* Sequencer_, UMovieSceneSequence* Sequence, FString FilePath);
	void SetControlRigTrack(UMovieSceneTrack* MovieSceneTrack_);
	void CastChannel();

	void InitKeys();

	void DeleteAllKeysFrom(FFrameNumber InTime);
	void DeleteKeysWithin(TRange<FFrameNumber> InRange);

	double GetValueFromTime(FFrameNumber InTime);

	void Optimize(TRange<FFrameNumber> InRange);

	bool IsValidMotionHandler();

	const IKeyArea* KeyArea;

	ISequencer* Sequencer;
	UMovieScene* MovieScene;
	UMovieSceneTrack* MovieSceneTrack;
	UMovieSceneControlRigParameterTrack* MovieSceneControlRigParameterTrack;

	FGuid GetObjectFGuid();
	Mode GetSelectedMode();
	void SetSelectedMode(Mode Mode_);
	bool SaveData();
	bool DeleteData();

private:
	void UpdateUI(FFrameNumber InTime);

	void SyncControlRigWithChannelValue(FFrameNumber InTime);

	FMotionHandlerData Data;

	FMovieSceneChannelHandle ChannelHandle;
	FMovieSceneFloatChannel* FloatChannel;
	FMovieSceneDoubleChannel* DoubleChannel;
	FMovieSceneBoolChannel* BoolChannel;
	FMovieSceneIntegerChannel* IntegerChannel;
};
