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
#include "LevelEditor.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "MotionHandler.h"
#include "MovieScene.h"
#include "MovieSceneBinding.h"
#include "SlateFwd.h"
#include "Styling/SlateTypes.h"
#include "Templates/SharedPointerInternals.h"
#include "Widgets/Input/SComboBox.h"
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
	void ChangeSelectedSequence(ULevelSequence* Sequence_);
	TSharedRef<SWidget> MakeSequenceWidget(ULevelSequence*);
	void OnSequenceSelected(ULevelSequence* sequence, ESelectInfo::Type SelectInfo);
	FText GetSelectedSequenceName() const;
	FReply OnRefreshSequencesClicked();

	bool IsKeysEnabled = false;

	void RefreshSequencer();
	FReply OnRefreshSequencer();

	FReply OnRefreshBindings();

	bool IsRecordedStarted;

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
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnCloseEvent, TSharedRef<ISequencer>);
	FOnCloseEvent* OnCloseEvent;
	bool IsSequencerRelevant;
	void OnCloseEventRaw(TSharedRef<ISequencer> Sequencer_);

	FSlateApplication::FOnApplicationPreInputKeyDownListener* OnKeyDownEvent;
	void OnKeyDownGlobal(const FKeyEvent& event);

	FVector2D PreviousPosition;

	FReply SelectX();
	FReply SelectXInverted();
	FReply SelectY();
	FReply SelectYInverted();

	TSharedPtr<SListView<TSharedPtr<MotionHandler>>> ListViewWidget;
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<MotionHandler> Item, const TSharedRef<STableViewBase>& OwnerTable);

	TArray<TSharedPtr<MotionHandler>> MotionHandlers;
	void AddMotionHandlers();
	void ExecuteMotionHandlers(bool isInTickMode);
	void LoadMotionHandlersFromDisk(TArray<TSharedPtr<MotionHandler>>& handlers);

	float fps = 24;
	double TimeFromLatestTestExecution;
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	float DefaultScale = 0.05;

	bool IsCustomRange = false;
	ECheckBoxState GetIsCustomRange() const;
	void OnIsCustomRangeChanged(ECheckBoxState NewState);
	TRange<FFrameNumber> GetCurrentRange() const;
	FText GetCustomStartFromFrame() const;
	FText GetCustomEndFrame() const;

private:
	TRange<FFrameNumber> CustomRange;
	TArray<ULevelSequence*> Sequences;
	ULevelSequence* SelectedSequence;
	TSharedPtr<ISequencer> Sequencer;
};
