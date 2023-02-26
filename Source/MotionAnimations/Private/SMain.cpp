// Fill out your copyright notice in the Description page of Project Settings.

#include "SMain.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Channels/MovieSceneBoolChannel.h"
#include "Channels/MovieSceneChannelEditorData.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"
#include "Containers/Array.h"
#include "CoreFwd.h"
#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "Editor/ContentBrowser/Public/ContentBrowserModule.h"
#include "Editor/Sequencer/Public/IKeyArea.h"
#include "Editor/Sequencer/Public/ISequencer.h"
#include "Editor/Sequencer/Public/SequencerKeyParams.h"
#include "Engine/AssetManager.h"
#include "Engine/EngineTypes.h"
#include "Framework/SlateDelegates.h"
#include "ILevelSequenceEditorToolkit.h"
#include "Input/Reply.h"
#include "Kismet2/EnumEditorUtils.h"
#include "LevelSequence.h"
#include "Logging/LogMacros.h"
#include "Logging/LogVerbosity.h"
#include "Misc/FileHelper.h"
#include "Misc/FrameNumber.h"
#include "Misc/Paths.h"
#include "Misc/QualifiedFrameTime.h"
#include "MotionAnimations.h"
#include "MotionHandler.h"
#include "MotionHandlerData.h"
#include "MovieScene.h"
#include "MovieSceneBinding.h"
#include "MovieSceneSection.h"
#include "MovieSceneSequence.h"
#include "SSettingsWidget.h"
#include "Sequencer/Public/SequencerAddKeyOperation.h"
#include "SequencerAddKeyOperation.h"
#include "Settings.h"
#include "SlateFwd.h"
#include "SlateOptMacros.h"
#include "Styling/SlateTypes.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Templates/SharedPointer.h"
#include "Types/SlateEnums.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <typeinfo>

SMain::SMain()
{
	IsStarted = false;	  // for execute motion handlers on tick

	Settings = new FSettings();

	OnGlobalTimeChangedDelegate = nullptr;
	OnPlayEvent = nullptr;
	OnStopEvent = nullptr;
	OnCloseEvent = nullptr;
	OnKeyDownEvent = nullptr;

	MotionHandlers = TArray<TSharedPtr<MotionHandler>>();
	CustomRange = TRange<FFrameNumber>();
	CustomRange.SetUpperBound(FFrameNumber());
	CustomRange.SetLowerBound(FFrameNumber());

	RefreshSequences();

	FSlateApplication& app = FSlateApplication::Get();
	OnKeyDownEvent = &(app.OnApplicationPreInputKeyDownListener());
	OnKeyDownEvent->AddRaw(this, &SMain::OnKeyDownGlobal);
}
SMain::~SMain()
{
	delete Settings;
	if (Sequencer != nullptr)
	{
		if (OnGlobalTimeChangedDelegate != nullptr)
		{
			if (OnGlobalTimeChangedDelegate != nullptr)
			{
				OnGlobalTimeChangedDelegate->RemoveAll(this);
			}
		}
		if (OnPlayEvent != nullptr)
		{
			if (OnPlayEvent != nullptr)
			{
				OnPlayEvent->RemoveAll(this);
			}
		}
		if (OnStopEvent != nullptr)
		{
			if (OnStopEvent != nullptr)
			{
				OnStopEvent->RemoveAll(this);
			}
		}
		if (OnCloseEvent != nullptr)
		{
			if (OnCloseEvent != nullptr)
			{
				OnCloseEvent->RemoveAll(this);
			}
		}
		if (OnKeyDownEvent != nullptr)
		{
			if (OnKeyDownEvent != nullptr)
			{
				OnKeyDownEvent->RemoveAll(this);
			}
		}
	}
}
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION void SMain::Construct(const FArguments& InArgs)
{
	ChildSlot
		[SNew(SScrollBox) +
			SScrollBox::Slot()[SNew(SHorizontalBox) +
							   SHorizontalBox::Slot()[SNew(STextBlock).Text(this, &SMain::GetIsActive)].Padding(
								   FMargin(10.f, 20.0f, 0.0f, 0.f)) +
							   SHorizontalBox::Slot()
								   [SNew(SHorizontalBox) +
									   SHorizontalBox::Slot()[SNew(SButton)
																  .Content()[SNew(STextBlock).Text(FText::FromString("Settings"))]
																  .OnClicked(this, &SMain::OpenSettingsWindow)]
										   .FillWidth(0.3f)
										   .HAlign(HAlign_Right)
										   .Padding(10.f)]] +
			SScrollBox::Slot()[SNew(SHorizontalBox) +
							   SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Current sequence:"))]
								   .AutoWidth()
								   .Padding(FMargin(0.f, 2.5f, 5.f, 0.f))
								   .AutoWidth() +
							   SHorizontalBox::Slot()[SNew(SComboBox<ULevelSequence*>)
														  .OptionsSource((&Sequences))
														  .OnGenerateWidget(this, &SMain::MakeSequenceWidget)
														  .OnSelectionChanged(this, &SMain::OnSequenceSelected)
														  .Content()[SNew(STextBlock).Text(this, &SMain::GetSelectedSequenceName)]]
								   .AutoWidth() +
							   SHorizontalBox::Slot()[SNew(SButton)
														  .Content()[SNew(STextBlock).Text(FText::FromString("Refresh Sequences"))]
														  .OnClicked(this, &SMain::OnRefreshSequencesClicked)]
								   .FillWidth(0.3f)
								   .HAlign(HAlign_Right)
								   .Padding(FMargin(0.f, 0.f, 10.f, 0.0f))]
				.Padding(FMargin(10.f, 0.f, 0.0f, 5.f)) +
			SScrollBox::Slot()
				[SNew(SVerticalBox) +
					SVerticalBox::Slot()[SNew(SHorizontalBox) +
										 SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Activate custom range"))]
											 .AutoWidth()
											 .Padding(FMargin(0.0f, 0.0f, 10.f, 0.0f)) +
										 SHorizontalBox::Slot()[SNew(SCheckBox)
																	.IsChecked(this, &SMain::GetIsCustomRange)
																	.OnCheckStateChanged(this, &SMain::OnIsCustomRangeChanged)

	]
											 .AutoWidth() +
										 SHorizontalBox::Slot()
											 [SNew(SHorizontalBox) +
												 SHorizontalBox::Slot()[SNew(STextBlock)
																			.Text(FText::FromString("Set optimization tolerance"))]
													 .Padding(FMargin(0.0f, 2.5f, 2.5f, 0.0f))
													 .AutoWidth() +
												 SHorizontalBox::Slot()[SNew(SSpinBox<double>)
																			.Value(this->OptimizationTolerance)
																			.OnValueChanged(this, &SMain::OnToleranceChangeRaw)]]
												 .FillWidth(0.3f)
												 .Padding(FMargin(0.f, 0.f, 10.f, 0.0f))
												 .HAlign(HAlign_Right)]
						.AutoHeight() +
					SVerticalBox::Slot()[SNew(STextBlock).Text(this, &SMain::GetCustomStartFromFrame)] +
					SVerticalBox::Slot()[SNew(STextBlock).Text(this, &SMain::GetCustomEndFrame)]]
					.Padding(FMargin(10.f, 0.f, 0.f, 5.f)) +
			SScrollBox::Slot()[SNew(SHorizontalBox) + SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Name"))] +
							   SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Scale"))] +
							   SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Current index"))] +
							   SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Selected Mode"))] +
							   SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Lower bound value"))] +
							   SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Upper bound value"))]]
				.Padding(FMargin(10.f, 5.f, 0.f, 5.f)) +
			SScrollBox::Slot()[SAssignNew(ListViewWidget, SListView<TSharedPtr<MotionHandler>>)
								   .ItemHeight(24)
								   .ListItemsSource(&MotionHandlers)
								   .OnGenerateRow(this, &SMain::OnGenerateRowForList)]];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SMain::MakeSequenceWidget(ULevelSequence* InSequence)
{
	return SNew(STextBlock).Text(FText::FromString(InSequence->GetDisplayName().ToString()));
}

TSharedRef<ITableRow> SMain::OnGenerateRowForList(TSharedPtr<MotionHandler> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<MotionHandler>>, OwnerTable)
		.Padding(2.0f)
		.Content()
			[SNew(SHorizontalBox) +
				SHorizontalBox::Slot()[SNew(SEditableText)
										   .Text(Item->Data.CustomName)
										   .OnTextChanged(Item->OnTextChanged)
										   .ClearKeyboardFocusOnCommit(true)]

				+ SHorizontalBox::Slot()[SNew(SSpinBox<double>).Value(Item->Data.Scale).OnValueChanged(Item->OnScaleValueChanged)] +
				SHorizontalBox::Slot()
					[SNew(SSpinBox<int32>).Value(Item->Data.CurrentIndex).OnValueChanged(Item->OnCurrentIndexValueChanged)] +
				SHorizontalBox::Slot()[SNew(STextBlock)
										   .Text(UEnum::GetDisplayValueAsText(Item->Data.SelectedMode))
										   .Justification(ETextJustify::Center)] +
				SHorizontalBox::Slot()
					[SNew(SSpinBox<double>).Value(Item->Data.LowerBoundValue).OnValueChanged(Item->OnLowerBoundValueChanged)] +
				SHorizontalBox::Slot()
					[SNew(SSpinBox<double>).Value(Item->Data.UpperBoundValue).OnValueChanged(Item->OnUpperBoundValueChanged)]];
}

FText SMain::GetIsActive() const
{
	if (IsKeysEnabled)
	{
		return FText::FromString("Active");
	}
	return FText::FromString("Not active");
};
void SMain::OnSequenceSelected(ULevelSequence* Sequence, ESelectInfo::Type SelectInfo)
{
	ChangeSelectedSequence(Sequence);
}
FReply SMain::OnRefreshSequencesClicked()
{
	RefreshSequences();
	return FReply::Handled();
}

FReply SMain::OpenSettingsWindow()
{
	TSharedRef<SWindow> SettingsWindow = SNew(SWindow).Title(FText::FromString(TEXT("Settings"))).ClientSize(FVector2D(400, 500));
	TSharedRef<SSettingsWidget> SettingsWidget = SNew(SSettingsWidget);
	SettingsWidget.Get().OnChangeKeyEvent->AddRaw(this, &SMain::RefreshSettings);
	SettingsWindow->SetContent(SettingsWidget);
	FSlateApplication::Get().AddWindow(SettingsWindow);
	return FReply::Handled();
}
void SMain::RefreshSettings()
{
	Settings->LoadSettingsFromDisk();
}

FText SMain::GetSelectedSequenceName() const
{
	if (SelectedSequence != nullptr)
	{
		if (SelectedSequence->IsValidLowLevel())
		{
			return SelectedSequence->GetDisplayName();
		}
	}
	return FText();
}

ECheckBoxState SMain::GetIsCustomRange() const
{
	return IsCustomRange ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SMain::OnIsCustomRangeChanged(ECheckBoxState NewState)
{
	IsCustomRange = !IsCustomRange;
}
TRange<FFrameNumber> SMain::GetCurrentRange() const
{
	if (IsCustomRange)
	{
		return CustomRange;
	}
	else
	{
		if (SelectedSequence != nullptr)
		{
			return SelectedSequence->GetMovieScene()->GetPlaybackRange();
		}
	}
	return TRange<FFrameNumber>();
}
FText SMain::GetCustomStartFromFrame() const
{
	if (IsCustomRange)
	{
		FFrameNumber lowerValue = GetCurrentRange().GetLowerBoundValue();
		FString string = FString("Start frame: ");
		FString value = FString::SanitizeFloat(lowerValue.Value);
		string.Append(value);
		return FText::FromString(string);
	}
	return FText::FromString("Custom start frame:");
}
FText SMain::GetCustomEndFrame() const
{
	if (IsCustomRange)
	{
		FFrameNumber upper = GetCurrentRange().GetUpperBoundValue();
		FString string = FString("End frame: ");
		FString value = FString::SanitizeFloat(upper.Value);
		string.Append(value);
		return FText::FromString(string);
	}
	return FText::FromString("Custom end frame:");
}

FReply SMain::OnRefreshSequencer()
{
	RefreshSequencer();
	return FReply::Handled();
}

void SMain::RefreshSequencer()
{
	if (IsSequencerRelevant)
	{
		return;
	}
	if (SelectedSequence != nullptr)
	{
		Sequencer = nullptr;
		UAssetEditorSubsystem* UAssetEditorSubs = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		IAssetEditorInstance* AssetEditor = UAssetEditorSubs->FindEditorForAsset(SelectedSequence, false);
		ILevelSequenceEditorToolkit* LevelSequenceEditor = (ILevelSequenceEditorToolkit*) AssetEditor;
		if (LevelSequenceEditor != nullptr)
		{
			// Get current Sequencer
			ISequencer* seq = LevelSequenceEditor->GetSequencer().Get();
			if (seq != nullptr)
			{
				Sequencer = seq;
				OnGlobalTimeChangedDelegate = &(Sequencer->OnGlobalTimeChanged());
				OnGlobalTimeChangedDelegate->AddRaw(this, &SMain::OnGlobalTimeChanged);
				OnPlayEvent = &(Sequencer->OnPlayEvent());
				OnPlayEvent->AddRaw(this, &SMain::OnStartPlay);
				OnStopEvent = &(Sequencer->OnStopEvent());
				OnStopEvent->AddRaw(this, &SMain::OnStopPlay);
				OnCloseEvent = &(Sequencer->OnCloseEvent());
				OnCloseEvent->AddRaw(this, &SMain::OnCloseEventRaw);

				LoadMotionHandlersFromDisk(MotionHandlers);
				ListViewWidget->RequestListRefresh();
				IsSequencerRelevant = true;
			};
		}
	}
}
void SMain::OnCloseEventRaw(TSharedRef<ISequencer> Sequencer_)
{
	IsSequencerRelevant = false;
}
void SMain::RefreshSequences()
{
	Sequences = TArray<ULevelSequence*>();
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> SequenceAssets;
	AssetRegistryModule.Get().GetAssetsByClass(ULevelSequence::StaticClass()->GetFName(), SequenceAssets);

	for (const FAssetData& AssetData : SequenceAssets)
	{
		ULevelSequence* Sequence = Cast<ULevelSequence>(AssetData.GetAsset());
		if (Sequence)
		{
			Sequences.Add(Sequence);	// Do something with the sequence here
		}
	}
	if (Sequences.Num() > 0)
	{
		ChangeSelectedSequence(Sequences[0]);
	}
}
void SMain::ChangeSelectedSequence(ULevelSequence* Sequence_)
{
	if (Sequence_ != nullptr)
	{
		SelectedSequence = Sequence_;
		IsSequencerRelevant = false;
	}
}
FReply SMain::OnRefreshBindings()
{
	AddMotionHandlers();
	return FReply::Handled();
};
void SMain::AddMotionHandlers()
{
	if (Sequencer != nullptr && SelectedSequence != nullptr)
	{
		TArray<const IKeyArea*> KeyAreas = TArray<const IKeyArea*>();
		Sequencer->GetSelectedKeyAreas(KeyAreas);
		TArray<FGuid> SelectedObjects = TArray<FGuid>();
		Sequencer->GetSelectedObjects(SelectedObjects);
		TArray<UMovieSceneTrack*> SelectedTracks = TArray<UMovieSceneTrack*>();
		Sequencer->GetSelectedTracks(SelectedTracks);
		if (SelectedObjects.Num() < 1 || SelectedTracks.Num() < 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("You are not selected any tracks or objects"));
			return;
		}
		/* UMovieSceneControlRigParameterTrack* controlRigTrack =
		Cast<UMovieSceneControlRigParameterTrack>(SelectedTracks[0]); if
		(IsValid(conolRigTrack))
		{
		  UE_LOG(LogTemp, Warning, TEXT("IT'S CONTROL rig TRACK!"));
		} */
		TArray<TSharedPtr<MotionHandler>> selectionSet;
		for (const IKeyArea* KeyArea : KeyAreas)
		{
			bool IsObjectAlreadyAdded = false;

			TSharedPtr<MotionHandler> motionHandler = TSharedPtr<MotionHandler>(new MotionHandler(
				KeyArea, DefaultScale, Sequencer, SelectedSequence, SelectedTracks[0], SelectedObjects[0], Mode::X));

			for (TSharedPtr<MotionHandler> handler : MotionHandlers)
			{
				if (*handler == *motionHandler)
				{
					IsObjectAlreadyAdded = true;
					selectionSet.Add(handler);
				}
			}
			if (!IsObjectAlreadyAdded)
			{
				MotionHandlers.Add(motionHandler);
				selectionSet.Add(motionHandler);
			}
			ListViewWidget->ClearSelection();
			ListViewWidget->SetItemSelection(selectionSet, true, ESelectInfo::Type::Direct);
			ListViewWidget->RequestListRefresh();
		}
	}
}
void SMain::LoadMotionHandlersFromDisk(TArray<TSharedPtr<MotionHandler>>& handlers)
{
	if (Sequencer != nullptr && SelectedSequence != nullptr)
	{
		FString First = FPaths::ProjectPluginsDir();
		FString PluginName_ = PluginName;
		FString SequenceName = SelectedSequence->GetDisplayName().ToString();
		FString SavesDir = FPaths::Combine(First, PluginName_, FString("Saved"), SequenceName);
		TArray<FString> FilePaths = TArray<FString>();
		FString FilesExtension = "";
		IFileManager::Get().FindFiles(FilePaths, *SavesDir, *FilesExtension);
		for (FString filename : FilePaths)
		{
			FString path = FPaths::Combine(SavesDir, filename);
			TSharedPtr<MotionHandler> handler = TSharedPtr<MotionHandler>(new MotionHandler(Sequencer, SelectedSequence, path));
			if (handler->IsValidMotionHandler())
			{
				bool IsObjectAlreadyAdded = false;
				for (TSharedPtr<MotionHandler> motionHandler : MotionHandlers)
				{
					if (*handler == *motionHandler)
					{
						IsObjectAlreadyAdded = true;
					}
				}
				if (!IsObjectAlreadyAdded)
				{
					MotionHandlers.Add(handler);
					ListViewWidget->RequestListRefresh();
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Motion handler is not valid"));
				handler->DeleteData();
			}
		}
	}
}
void SMain::OnGlobalTimeChanged()
{
	FVector2D currentPosition = FSlateApplication::Get().GetCursorPos();
	FVector2D vectorChange = PreviousPosition - currentPosition;
	if (IsRecordedStarted)
	{
		if (IsCustomRange)
		{
			FFrameNumber CurrentTime = Sequencer->GetLocalTime().Time.FrameNumber;
			if (CurrentTime.Value >= CustomRange.GetLowerBoundValue().Value &&
				CurrentTime.Value <= CustomRange.GetUpperBoundValue().Value)
			{
				ExecuteMotionHandlers(vectorChange);
			}
		}
		else
		{
			ExecuteMotionHandlers(vectorChange);
		}
	}
	if (IsScalingStarted)
	{
		if (MotionHandlers.Num() > 0)
		{
			FFrameNumber nextFrame = Sequencer->GetGlobalTime().Time.GetFrame();
			nextFrame.Value += 1000;
			for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
			{
				motionHandler->Accelerate(vectorChange, nextFrame);
			}
		}
	}
	PreviousPosition = FSlateApplication::Get().GetCursorPos();
};
void SMain::ExecuteMotionHandlers(FVector2D value)
{
	FFrameNumber nextFrame = Sequencer->GetGlobalTime().Time.GetFrame();
	nextFrame.Value += 1000;
	if (MotionHandlers.Num() > 0)
	{
		for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
		{
			motionHandler->SetKey(nextFrame, value);
		}
	}
}
FReply SMain::SelectX()
{
	for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
	{
		motionHandler->SetSelectedMode(Mode::X);
	}
	return FReply::Handled();
}
FReply SMain::SelectXInverted()
{
	for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
	{
		motionHandler->SetSelectedMode(Mode::XInverted);
	}
	return FReply::Handled();
}
FReply SMain::SelectY()
{
	for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
	{
		motionHandler->SetSelectedMode(Mode::Y);
	}
	return FReply::Handled();
}
FReply SMain::SelectYInverted()
{
	for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
	{
		motionHandler->SetSelectedMode(Mode::YInverted);
	}
	return FReply::Handled();
}
void SMain::OnStartPlay()
{
	IsStarted = true;
}
void SMain::OnStopPlay()
{
	IsStarted = false;
	IsRecordedStarted = false;
	IsScalingStarted = false;
}
void SMain::OnKeyDownGlobal(const FKeyEvent& event)
{
	FString key = event.GetKey().ToString();
	if (Settings->Keys.Num() < 14)	  // there should be at least 14 settings
	{
		UE_LOG(LogTemp, Warning, TEXT("Something with your settings file!"));
		return;
	}
	if (Settings->Keys["Activate"] == key)
	{
		IsKeysEnabled = !IsKeysEnabled;
	}
	if (!IsKeysEnabled)
	{
		return;
	}
	if (Settings->Keys["Delete item"] == key)
	{
		for (TSharedPtr<MotionHandler> handler : ListViewWidget->GetSelectedItems())
		{
			handler->DeleteData();
			MotionHandlers.Remove(handler);
		}
	}
	if (Settings->Keys["Refresh sequencer"] == key)
	{
		RefreshSequencer();
		AddMotionHandlers();
		for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
		{
			motionHandler->ReInitAccelerator(GetCurrentRange());
		}
	}

	TRange<FFrameNumber> playbackRange = SelectedSequence->GetMovieScene()->GetPlaybackRange();
	FFrameNumber lowerValue = playbackRange.GetLowerBoundValue();
	FFrameNumber highValue = playbackRange.GetUpperBoundValue();
	auto stopSequencerAndBackToFirstFrame = [&]()
	{
		if (Sequencer != nullptr)
		{
			Sequencer->Pause();
			Sequencer->SetGlobalTime(lowerValue);
		}
	};
	auto playSequencerToLastFrame = [&]()
	{
		FMovieSceneSequencePlaybackParams params = FMovieSceneSequencePlaybackParams();
		params.Frame = highValue;
		Sequencer->PlayTo(params);
	};

	if (Settings->Keys["Start recording"] == key)
	{
		if (SelectedSequence != nullptr && Sequencer != nullptr)
		{
			stopSequencerAndBackToFirstFrame();

			TRange<FFrameNumber> CurrentRange_ = GetCurrentRange();
			FFrameNumber lowerCurrentValue = CurrentRange_.GetLowerBoundValue();
			FFrameNumber DeleteKeysTo = CurrentRange_.GetUpperBoundValue();

			for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
			{
				motionHandler->PreviousValue = (double) motionHandler->GetValueFromTime(lowerCurrentValue);
				FFrameNumber DeleteKeysFrom = lowerCurrentValue;
				DeleteKeysFrom.Value += 1000;
				DeleteKeysTo.Value += 2000;
				motionHandler->DeleteKeysWithin(TRange<FFrameNumber>(DeleteKeysFrom, DeleteKeysTo));
				motionHandler->ResetNiagaraState();
			}

			playSequencerToLastFrame();

			IsRecordedStarted = true;
			PreviousPosition = FSlateApplication::Get().GetCursorPos();

			Sequencer->UpdatePlaybackRange();
			Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::RefreshAllImmediately);
			Sequencer->NotifyBindingsChanged();
		}
		else
		{
			UE_LOG(LogTemp, Warning,
				TEXT("No sequence selected! Or you selected wrong sequence, not the one that is open in sequencer"));
		}
	}
	if (Settings->Keys["Start scaling"] == key)
	{
		stopSequencerAndBackToFirstFrame();

		TRange<FFrameNumber> CurrentRange_ = GetCurrentRange();
		FFrameNumber lowerCurrentValue = CurrentRange_.GetLowerBoundValue();
		FFrameNumber DeleteKeysTo = CurrentRange_.GetUpperBoundValue();

		for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
		{
			motionHandler->PreviousValue = (double) motionHandler->GetValueFromTime(lowerCurrentValue);
			lowerCurrentValue.Value += 1000;
			CurrentRange_.SetLowerBound(lowerCurrentValue);
			motionHandler->ResetAccelerator(CurrentRange_);
			motionHandler->ResetNiagaraState();
		}

		playSequencerToLastFrame();

		IsScalingStarted = true;
		PreviousPosition = FSlateApplication::Get().GetCursorPos();

		Sequencer->UpdatePlaybackRange();
		Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::RefreshAllImmediately);
		Sequencer->NotifyBindingsChanged();
	}
	if (Settings->Keys["Stop recording and optimize"] == key)
	{
		if (SelectedSequence != nullptr && Sequencer != nullptr)
		{
			TRange<FFrameNumber> CurrentRange_ = GetCurrentRange();
			FFrameNumber lowerCurrentValue = CurrentRange_.GetLowerBoundValue();
			FFrameNumber higherCurrentValue = CurrentRange_.GetUpperBoundValue();
			FFrameNumber lowerCurrentValueCopy = lowerCurrentValue;
			lowerCurrentValueCopy.Value += 1000;
			CurrentRange_.SetLowerBoundValue(lowerCurrentValueCopy);

			for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
			{
				motionHandler->Optimize(CurrentRange_, OptimizationTolerance);
				motionHandler->PreviousValue = (double) motionHandler->GetValueFromTime(lowerCurrentValue);
				motionHandler->ResetNiagaraState();
			}

			stopSequencerAndBackToFirstFrame();
		}
		else
		{
			UE_LOG(LogTemp, Warning,
				TEXT("No sequence selected! Or you selected wrong sequence, not the one that is open in sequencer"));
		}
	}
	if (Settings->Keys["Preview animation"] == key)
	{
		if (SelectedSequence != nullptr && Sequencer != nullptr)
		{
			stopSequencerAndBackToFirstFrame();

			for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
			{
				motionHandler->ResetNiagaraState();
			}

			playSequencerToLastFrame();

			Sequencer->UpdatePlaybackRange();
			Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::RefreshAllImmediately);
			Sequencer->NotifyBindingsChanged();
		}
	}
	if (Settings->Keys["Save item"] == key)
	{
		for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
		{
			motionHandler->AddOrUpdateKeyValueInSequencer();
			motionHandler->SaveData();
		}
	}
	if (Settings->Keys["Load keys from item to sequencer"] == key)
	{
		for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
		{
			motionHandler->InsertCurrentKeyValuesToSequencer();
		}
	}
	if (Settings->Keys["Select x axis"] == key)
	{
		SelectX();
		ListViewWidget->RebuildList();
	}
	else if (Settings->Keys["Select -x axis"] == key)
	{
		SelectXInverted();
		ListViewWidget->RebuildList();
	}
	else if (Settings->Keys["Select y axis"] == key)
	{
		SelectY();
		ListViewWidget->RebuildList();
	}
	else if (Settings->Keys["Select -y axis"] == key)
	{
		SelectYInverted();
		ListViewWidget->RebuildList();
	}
	else if (Settings->Keys["Set lower bound of custom range"] == key)
	{
		if (Sequencer != nullptr && SelectedSequence != nullptr)
		{
			CustomRange.SetLowerBound(Sequencer->GetLocalTime().Time.FrameNumber);
		}
	}
	else if (Settings->Keys["Set upper bound of custom range"] == key)
	{
		if (Sequencer != nullptr && SelectedSequence != nullptr)
		{
			CustomRange.SetUpperBound(Sequencer->GetLocalTime().Time.FrameNumber);
		}
	}
}
void SMain::OnToleranceChangeRaw(double value)
{
	OptimizationTolerance = value;
}
