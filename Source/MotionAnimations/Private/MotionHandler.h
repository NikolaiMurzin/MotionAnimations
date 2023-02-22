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
#include "MovieScene/Parameters/MovieSceneNiagaraFloatParameterTrack.h"
#include "MovieScene/Parameters/MovieSceneNiagaraParameterTrack.h"
#include "MovieScene/Parameters/MovieSceneNiagaraVectorParameterTrack.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sequencer/MovieSceneControlRigParameterTrack.h"
#include "Sequencer/Public/ISequencer.h"
#include "Tracks/MovieSceneMaterialTrack.h"
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
	void SetMaterialTrack(UMovieSceneTrack* MovieSceneTrack_);
	void SetNiagaraTrack(UMovieSceneTrack* NiagaraTrack_);
	void CastChannel();

	void DeleteAllKeysFrom(FFrameNumber InTime);
	void DeleteKeysWithin(TRange<FFrameNumber> InRange);

	double GetValueFromTime(FFrameNumber InTime);

	void Optimize(TRange<FFrameNumber> InRange);

	bool IsValidMotionHandler();
	bool ValidMotionHandler;	// for check latest execute of function IsValidMotionHandler();

	const IKeyArea* KeyArea;

	ISequencer* Sequencer;
	UMovieScene* MovieScene;
	UMovieSceneTrack* MovieSceneTrack;

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

	DECLARE_DELEGATE_OneParam(FOnUpperBoundValueChanged, double);
	FOnUpperBoundValueChanged OnUpperBoundValueChanged;
	void OnUpperBoundValueChangedRaw(double value);

	DECLARE_DELEGATE_OneParam(FOnLowerBoundValueChanged, double);
	FOnLowerBoundValueChanged OnLowerBoundValueChanged;
	void OnLowerBoundValueChangedRaw(double value);

	DECLARE_DELEGATE_OneParam(FOnTextChanged, const FText& /*NewValue*/);
	FOnTextChanged OnTextChanged;
	void OnTextChangedRaw(const FText& value);

	FMovieSceneChannelHandle ChannelHandle;

	FMotionHandlerData Data;

	void ResetNiagaraState();

private:
	void SyncControlRigWithChannelValue(FFrameNumber InTime);
	void SyncMaterialTrack(FFrameNumber InTime);
	void SyncNiagaraParam(FFrameNumber InTime);

	UMovieSceneControlRigParameterTrack* MovieSceneControlRigParameterTrack = nullptr;
	UMovieSceneMaterialTrack* MovieSceneMaterialTrack = nullptr;
	UMovieSceneNiagaraTrack* MovieSceneNiagaraTrack = nullptr;
	UMovieSceneNiagaraParameterTrack* MovieSceneNiagaraParameterTrack = nullptr;
	UMovieSceneNiagaraFloatParameterTrack* MovieSceneNiagaraFloatTrack = nullptr;
	UMovieSceneNiagaraVectorParameterTrack* MovieSceneNiagaraVectorTrack = nullptr;

	UNiagaraComponent* NiagaraComponent = nullptr;

	FMovieSceneFloatChannel* FloatChannel;
	FMovieSceneDoubleChannel* DoubleChannel;
	FMovieSceneBoolChannel* BoolChannel;
	FMovieSceneIntegerChannel* IntegerChannel;
};
