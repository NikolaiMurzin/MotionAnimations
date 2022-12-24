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
#include "Misc/Paths.h"
#include "Misc/QualifiedFrameTime.h"
#include "MotionHandler.h"
#include "MotionHandlerData.h"
#include "MouseAnimations.h"
#include "MovieScene.h"
#include "MovieSceneBinding.h"
#include "MovieSceneSection.h"
#include "STreeViewSequencer.h"
#include "Sequencer/Public/SequencerAddKeyOperation.h"
#include "SequencerAddKeyOperation.h"
#include "SlateFwd.h"
#include "SlateOptMacros.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Templates/SharedPointer.h"
#include "Types/SlateEnums.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <typeinfo>

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION void SMain::Construct(const FArguments& InArgs)
{
	IsStarted = false;	  // for execute motion handlers on tick
	MotionHandlers = TArray<TSharedPtr<MotionHandler>>();

	RefreshSequences();
	if (Sequences.Num() > 0)
	{
		ChangeSelectedSequence(Sequences[0]);
	}
	RefreshSequencer();

	FSlateApplication& app = FSlateApplication::Get();
	OnKeyDownEvent = &(app.OnApplicationPreInputKeyDownListener());
	OnKeyDownEvent->AddRaw(this, &SMain::OnKeyDownGlobal);

	ChildSlot[SNew(SScrollBox) +
			  SScrollBox::Slot()[SNew(SButton).Content()[SNew(STextBlock).Text(FText::FromString("Refresh Sequences"))].OnClicked(
				  this, &SMain::OnRefreshSequencesClicked)] +
			  SScrollBox::Slot()[SNew(SComboBox<ULevelSequence*>)
									 .OptionsSource((&Sequences))
									 .OnGenerateWidget(this, &SMain::MakeSequenceWidget)
									 .OnSelectionChanged(this, &SMain::OnSequenceSelected)
									 .Content()[SNew(STextBlock).Text(this, &SMain::GetSelectedSequenceName)]] +
			  SScrollBox::Slot()[SAssignNew(ListViewWidget, SListView<TSharedPtr<MotionHandler>>)
									 .ItemHeight(24)
									 .ListItemsSource(&MotionHandlers)
									 .OnGenerateRow(this, &SMain::OnGenerateRowForList)]];

	ListViewWidget->SetListItemsSource(MotionHandlers);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SMain::MakeSequenceWidget(ULevelSequence* InSequence)
{
	return SNew(STextBlock).Text(FText::FromString(InSequence->GetDisplayName().ToString()));
}
void SMain::OnSequenceSelected(ULevelSequence* Sequence, ESelectInfo::Type SelectInfo)
{
	ChangeSelectedSequence(Sequence);
}
FReply SMain::OnRefreshSequencesClicked()
{
	RefreshSequences();
	return FReply::Handled();
}

FText SMain::GetSelectedSequenceName() const
{
	if (SelectedSequence != nullptr)
	{
		return SelectedSequence->GetDisplayName();
	}
	return FText();
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
										   .ClearKeyboardFocusOnCommit(true)] +
				SHorizontalBox::Slot()[SNew(SSpinBox<double>).Value(Item->Data.Scale).OnValueChanged(Item->OnScaleValueChanged)] +
				SHorizontalBox::Slot()
					[SNew(SSpinBox<int32>).Value(Item->Data.CurrentIndex).OnValueChanged(Item->OnCurrentIndexValueChanged)] +
				SHorizontalBox::Slot()[SNew(STextBlock).Text(UEnum::GetDisplayValueAsText(Item->Data.SelectedMode))]];
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
		UAssetEditorSubsystem* UAssetEditorSubs = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		IAssetEditorInstance* AssetEditor = UAssetEditorSubs->FindEditorForAsset(SelectedSequence, false);
		ILevelSequenceEditorToolkit* LevelSequenceEditor = (ILevelSequenceEditorToolkit*) AssetEditor;
		if (LevelSequenceEditor != nullptr)
		{
			// Get current Sequencer
			ISequencer* seq = LevelSequenceEditor->GetSequencer().Get();
			if (seq != nullptr)
			{
				Sequencer = TSharedPtr<ISequencer>(seq);
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
	if (Sequencer != nullptr)
	{
		TArray<const IKeyArea*> KeyAreas = TArray<const IKeyArea*>();
		Sequencer->GetSelectedKeyAreas(KeyAreas);
		TArray<FGuid> SelectedObjects = TArray<FGuid>();
		Sequencer->GetSelectedObjects(SelectedObjects);
		TArray<UMovieSceneTrack*> SelectedTracks = TArray<UMovieSceneTrack*>();
		Sequencer->GetSelectedTracks(SelectedTracks);
		/* UMovieSceneControlRigParameterTrack* controlRigTrack =
		Cast<UMovieSceneControlRigParameterTrack>(SelectedTracks[0]); if
		(IsValid(controlRigTrack))
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
			}
		}
	}
}
void SMain::OnGlobalTimeChanged()
{
	if (IsRecordedStarted)
	{
		ExecuteMotionHandlers(false);
	}
};
void SMain::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (false)	  // keep that code just to remember how to execute motion handlers
				  // every tick
	{
		if (IsStarted)
		{
			float desiredDeltaTime = 1 / (fps * 3);
			float timeFromLatest = (float) (InCurrentTime - TimeFromLatestTestExecution);
			FVector2D cursorPos = FSlateApplication::Get().GetCursorPos();

			if (timeFromLatest >= desiredDeltaTime)
			{
				ExecuteMotionHandlers(true);
			}
		}
	}
}
void SMain::ExecuteMotionHandlers(bool isInTickMode)
{
	FFrameNumber nextFrame = Sequencer->GetGlobalTime().Time.GetFrame();
	nextFrame.Value += 1000;
	FVector2D currentPosition = FSlateApplication::Get().GetCursorPos();
	FVector2D vectorChange = PreviousPosition - currentPosition;
	if (MotionHandlers.Num() > 0)
	{
		for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
		{
			motionHandler->SetKey(nextFrame, vectorChange);
		}
	}
	PreviousPosition = FSlateApplication::Get().GetCursorPos();
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
	UE_LOG(LogTemp, Warning, TEXT("on start play"));
	IsStarted = true;
}
void SMain::OnStopPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("on stop play"));
	IsStarted = false;
	IsRecordedStarted = false;
}
void SMain::OnKeyDownGlobal(const FKeyEvent& event)
{
	FKey key = event.GetKey();
	if (key.ToString() == "F2")
	{
		IsKeysEnabled = !IsKeysEnabled;
	}
	if (!IsKeysEnabled)
	{
		return;
	}
	if (key.ToString() == "F11")
	{
		for (TSharedPtr<MotionHandler> handler : ListViewWidget->GetSelectedItems())
		{
			handler->DeleteData();
			MotionHandlers.Remove(handler);
		}
	}
	if (key.ToString() == "W")
	{
		RefreshSequencer();
		AddMotionHandlers();
	}
	if (key.ToString() == "E")
	{
		if (SelectedSequence != nullptr && Sequencer != nullptr)
		{
			TRange<FFrameNumber> playbackRange = SelectedSequence->GetMovieScene()->GetPlaybackRange();

			FFrameNumber lowerValue = playbackRange.GetLowerBoundValue();
			FFrameNumber highValue = playbackRange.GetUpperBoundValue();

			Sequencer->Pause();
			Sequencer->SetGlobalTime(lowerValue);

			for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
			{
				motionHandler->PreviousValue = (double) motionHandler->GetValueFromTime(lowerValue);

				FFrameNumber DeleteKeysFrom = lowerValue;
				DeleteKeysFrom.Value += 1000;
				motionHandler->DeleteKeysWithin(TRange<FFrameNumber>(DeleteKeysFrom, highValue));
			}
			FMovieSceneSequencePlaybackParams params = FMovieSceneSequencePlaybackParams();
			params.Frame = highValue;
			Sequencer->PlayTo(params);
			IsRecordedStarted = true;
			PreviousPosition = FSlateApplication::Get().GetCursorPos();
		}
	}
	if (key.ToString() == "D")
	{
		if (SelectedSequence != nullptr && Sequencer != nullptr)
		{
			IsRecordedStarted = false;

			TRange<FFrameNumber> playbackRange = SelectedSequence->GetMovieScene()->GetPlaybackRange();
			FFrameNumber lowerValue = playbackRange.GetLowerBoundValue();
			FFrameNumber highValue = playbackRange.GetUpperBoundValue();

			PreviousPosition = FSlateApplication::Get().GetCursorPos();
			for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
			{
				motionHandler->Optimize(playbackRange);
				motionHandler->PreviousValue = (double) motionHandler->GetValueFromTime(lowerValue);
			}
			Sequencer->Pause();
			Sequencer->SetGlobalTime(lowerValue);
			FMovieSceneSequencePlaybackParams params = FMovieSceneSequencePlaybackParams();
			params.Frame = highValue;
		}
	}
	if (key.ToString() == "F5")
	{
		for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
		{
			motionHandler->AddOrUpdateKeyValueInSequencer();
			motionHandler->SaveData();
		}
	}
	if (key.ToString() == "F6")
	{
		for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
		{
			motionHandler->InsertCurrentKeyValuesToSequencer();
		}
	}
	if (key.ToString() == "F12")
	{
		for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
		{
			motionHandler->DeleteKeyValues();
		}
	}
	if (key.ToString() == "Z")
	{
		SelectX();
	}
	else if (key.ToString() == "X")
	{
		SelectXInverted();
	}
	else if (key.ToString() == "C")
	{
		SelectY();
	}
	else if (key.ToString() == "V")
	{
		SelectYInverted();
	}
}
