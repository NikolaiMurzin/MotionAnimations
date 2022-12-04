// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Channels/MovieSceneChannelHandle.h"
#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "ISequencer.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "MovieScene.h"
#include "MovieSceneBinding.h"
#include "Widgets/SCompoundWidget.h"

/**
 *
 */
class MOUSEANIMATIONS_API SMain : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMain)
	{
	}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	ULevelSequence* GetSequence() const;
	ULevelSequence* GetLevelSequenceFromWorld() const;
	void SetSequence();

	ISequencer* GetSequencer();
	ISequencer* GetSequencerFromSelectedSequence();
	void SetSequencer();

	FReply OnButtonClicked();

	void SetChannelHandleFromSelectedKeys();
	FMovieSceneChannelHandle ChannelHandle;
	FName ChannelType;

	void OnGlobalTimeChanged();

private:
	ULevelSequence* Sequence;
	ISequencer* Sequencer;
};
