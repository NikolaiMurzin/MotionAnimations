// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Channels/MovieSceneChannelHandle.h"
#include "Containers/Array.h"
#include "CoreFwd.h"
#include "CoreMinimal.h"
#include "Editor/Sequencer/Public/IKeyArea.h"
#include "Editor/Sequencer/Public/ISequencer.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "MotionHandler.h"
#include "MovieScene.h"
#include "MovieSceneBinding.h"
#include "Widgets/Input/SSpinBox.h"
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

	void RefreshSequences();

	void RefreshSequencer();
	FReply OnRefreshSequencer();

	FReply OnRefreshBindings();

	void SetChannelHandleFromSelectedKeys();
	FMovieSceneChannelHandle ChannelHandle;
	FName ChannelType;

	DECLARE_MULTICAST_DELEGATE(FOnGlobalTimeChanged)
	FOnGlobalTimeChanged* OnGlobalTimeChangedDelegate;
	void OnGlobalTimeChanged();

	TSharedPtr<SSpinBox<double>> DefaultScaleBox;
	FVector2D PreviousPosition;
	double PositionX;
	double PositionY;
	FReply OnToggleRecording();
	bool IsRecordedStarted;

	Mode SelectedMode;
	FReply SelectX();
	FReply SelectXInverted();
	FReply SelectY();
	FReply SelectYInverted();

	bool IsTestAnimations;
	FReply OnToggleTestAnimations();

	TSharedPtr<TArray<TSharedPtr<MotionHandler>>> MotionHandlerPtrs;
	void RefreshMotionHandlers();
	void ExecuteMotionHandlers();

	float fps = 24;
	double TimeFromLatestTestExecution;
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	TArray<ULevelSequence*> Sequences;
	ULevelSequence* SelectedSequence;
	ISequencer* Sequencer;
};
