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
#include "Math/NumericLimits.h"
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
	MotionHandler(const IKeyArea* KeyAreas, double DefaultScale_, TSharedPtr<ISequencer> Sequencer_, UMovieSceneSequence* Sequence_,
		UMovieSceneTrack* MovieSceneTrack_, FGuid ObjectFGuid_, Mode Mode_ = Mode::X);
	MotionHandler(TSharedPtr<ISequencer> Sequencer_, UMovieSceneSequence* Sequence, FString FilePath);
	void SetControlRigTrack(UMovieSceneTrack* MovieSceneTrack_);
	void CastChannel();

	void InitKeys();

	void DeleteAllKeysFrom(FFrameNumber InTime);
	void DeleteKeysWithin(TRange<FFrameNumber> InRange);

	double GetValueFromTime(FFrameNumber InTime);

	void Optimize(TRange<FFrameNumber> InRange);

	bool IsValidMotionHandler();
	bool ValidMotionHandler;	// for check latest execute of function IsValidMotionHandler();

	const IKeyArea* KeyArea;

	TSharedPtr<ISequencer> Sequencer;
	UMovieScene* MovieScene;
	UMovieSceneTrack* MovieSceneTrack;
	UMovieSceneControlRigParameterTrack* MovieSceneControlRigParameterTrack;

	FGuid GetObjectFGuid();
	Mode GetSelectedMode();
	FText GetSelectedModeText() const;
	void SetSelectedMode(Mode Mode_);

	bool SaveData();
	bool DeleteData();
	void AddOrUpdateKeyValueInSequencer();
	void AddNewKeyValues();
	void UpdateCurrentKeyValues();
	void DeleteKeyValues();
	void InsertCurrentKeyValuesToSequencer();

	bool operator==(MotionHandler& handler);

	DECLARE_DELEGATE_OneParam(FOnValueChanged, double /*NewValue*/);
	FOnValueChanged OnScaleValueChanged;
	void OnScaleValueChangedRaw(double value);

	DECLARE_DELEGATE_OneParam(FOnCurrentIndexValueChanged, int32);
	FOnCurrentIndexValueChanged OnCurrentIndexValueChanged;
	void OnCurrentIndexValueChangedRaw(int32 value);

	DECLARE_DELEGATE_OneParam(FOnTextChanged, const FText& /*NewValue*/);
	FOnTextChanged OnTextChanged;
	void OnTextChangedRaw(const FText& value);

	FMovieSceneChannelHandle ChannelHandle;

	FMotionHandlerData Data;

private:
	void SyncControlRigWithChannelValue(FFrameNumber InTime);

	FMovieSceneFloatChannel* FloatChannel;
	FMovieSceneDoubleChannel* DoubleChannel;
	FMovieSceneBoolChannel* BoolChannel;
	FMovieSceneIntegerChannel* IntegerChannel;
};
