// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Channels/MovieSceneChannelHandle.h"
#include "Containers/Array.h"
#include "CoreFwd.h"
#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "Editor/Sequencer/Public/IKeyArea.h"
#include "Editor/Sequencer/Public/ISequencer.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "MotionHandler.h"
#include "MovieScene.h"
#include "MovieSceneBinding.h"
#include "Templates/SharedPointerInternals.h"
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

	FReply OnToggleRecording();
	bool IsRecordedStarted;

	FText GetIsToggledRecording() const;

	void SetChannelHandleFromSelectedKeys();
	FMovieSceneChannelHandle ChannelHandle;
	FName ChannelType;

	DECLARE_MULTICAST_DELEGATE(FOnGlobalTimeChanged)
	FOnGlobalTimeChanged* OnGlobalTimeChangedDelegate;
	void OnGlobalTimeChanged();
	DECLARE_MULTICAST_DELEGATE(FOnStopEvent);
	FOnStopEvent* OnStopEvent;
	void OnStopPlay();
	DECLARE_MULTICAST_DELEGATE(FOnPlayEvent);
	FOnPlayEvent* OnPlayEvent;
	bool IsStarted;
	void OnStartPlay();

	FSlateApplication::FOnApplicationPreInputKeyDownListener* OnKeyDownEvent;
	void OnKeyDownGlobal(const FKeyEvent& event);

	TSharedPtr<SSpinBox<double>> DefaultScaleBox;
	FVector2D PreviousPosition;
	double PositionX;
	double PositionY;

	FReply SelectX();
	FReply SelectXInverted();
	FReply SelectY();
	FReply SelectYInverted();

	bool IsTestAnimations;
	FReply OnToggleTestAnimations();

	TSharedPtr<TArray<TSharedPtr<MotionHandler>>> MotionHandlerPtrs;
	void RefreshMotionHandlers();
	void ExecuteMotionHandlers(bool isInTickMode);

	double GetValueFromTime(FFrameNumber InTime);

	float fps = 24;
	double TimeFromLatestTestExecution;
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	TArray<ULevelSequence*> Sequences;
	ULevelSequence* SelectedSequence;
	ISequencer* Sequencer;
};
