// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

#pragma once

#include "Channels/MovieSceneChannelHandle.h"
#include "Containers/Array.h"
#include "Subsystems/AssetEditorSubsystem.h"
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
#include "Widgets/SWindow.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/SCompoundWidget.h"
#include "Settings.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "ISequencer.h"

/**
 *
 */
class MOTIONANIMATIONS_API SMain : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMain)
	{
	}
	SLATE_END_ARGS()

	SMain();
	~SMain();

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	FText GetIsActive() const;

	void RefreshSequence();
	void RefreshSequencer();
	void RefreshSettings();

	bool IsKeysEnabled = false;
	bool IsRecordedStarted;
	bool IsScalingStarted = false;
	bool IsEditStarted = false;

	UAssetEditorSubsystem::FAssetEditorOpenEvent* OpenEditorEvent;
	void OnEditorOpened(UObject* object);

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
	void OnCloseEventRaw(TSharedRef<ISequencer> Sequencer_);

	ISequencer::FOnSelectionChangedSections* SelectedSectionsChangedEvent;
	void SelectedSectionsChangedRaw(TArray<UMovieSceneSection*> sections);

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
	void ExecuteMotionHandlers(FVector2D value, FFrameNumber frame);
	void LoadMotionHandlersFromDisk(TArray<TSharedPtr<MotionHandler>>& handlers);

	float DefaultScale = 0.05;
	double PopulateValue = 1000;
	void OnPopulateValueChangedRaw(double val);

	bool IsCustomRange = false;
	ECheckBoxState GetIsCustomRange() const;
	void OnIsCustomRangeChanged(ECheckBoxState NewState);
	TRange<FFrameNumber> GetCurrentRange() const;
	FText GetCustomStartFromFrame() const;
	FText GetCustomEndFrame() const;

	FReply OpenSettingsWindow();

	void OnToleranceChangeRaw(double value);
	double OptimizationTolerance = 0.01;

private:
	TRange<FFrameNumber> CustomRange;
	ULevelSequence* SelectedSequence;
	FSettings* Settings;
	ISequencer* Sequencer;
};
