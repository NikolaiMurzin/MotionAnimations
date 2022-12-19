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
#include "Editor/Sequencer/Public/IKeyArea.h"
#include "Editor/Sequencer/Public/ISequencer.h"
#include "Editor/Sequencer/Public/SequencerKeyParams.h"
#include "Engine/EngineTypes.h"
#include "Framework/SlateDelegates.h"
#include "ILevelSequenceEditorToolkit.h"
#include "Input/Reply.h"
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
#include "SlateOptMacros.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Templates/SharedPointer.h"
#include "Types/SlateEnums.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SButton.h"
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
	RefreshSequences();
	ChangeSelectedSequence(Sequences[0]);
	RefreshSequencer();
	MotionHandlers = TArray<TSharedPtr<MotionHandler>>();

	FSlateApplication& app = FSlateApplication::Get();
	OnKeyDownEvent = &(app.OnApplicationPreInputKeyDownListener());
	OnKeyDownEvent->AddRaw(this, &SMain::OnKeyDownGlobal);

	ChildSlot[SNew(SScrollBox) + SScrollBox::Slot()[SAssignNew(ListViewWidget, SListView<TSharedPtr<MotionHandler>>)
														.ItemHeight(24)
														.ListItemsSource(&MotionHandlers)
														.OnGenerateRow(this, &SMain::OnGenerateRowForList)]];

	ListViewWidget->SetListItemsSource(MotionHandlers);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<ITableRow> SMain::OnGenerateRowForList(TSharedPtr<MotionHandler> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	FString trackName = Item->MovieSceneTrack->GetName();
	FString movieSceneName = Item->MovieScene->GetName();
	FMovieSceneChannelHandle channelHandle = Item->ChannelHandle;
	FString channelName = channelHandle.GetMetaData()->DisplayText.ToString();
	FString channelDisplayText = channelHandle.GetMetaData()->Name.ToString();
	FString group = channelHandle.GetMetaData()->Group.ToString();
	FString sortOrder = FString::FromInt(channelHandle.GetMetaData()->SortOrder);
	return SNew(STableRow<TSharedPtr<MotionHandler>>, OwnerTable)
		.Padding(2.0f)
		.Content()[SNew(STextBlock)
					   .Text(FText::FromString(trackName + ", " + channelDisplayText + ", " + group + ", " + sortOrder))];
}

FReply SMain::OnRefreshSequencer()
{
	RefreshSequencer();
	return FReply::Handled();
}

void SMain::RefreshSequencer()
{
	if (SelectedSequence != nullptr && !IsSequencerRelevant)
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
				Sequencer = seq;
				OnGlobalTimeChangedDelegate = &(Sequencer->OnGlobalTimeChanged());
				OnGlobalTimeChangedDelegate->AddRaw(this, &SMain::OnGlobalTimeChanged);
				OnPlayEvent = &(Sequencer->OnPlayEvent());
				OnPlayEvent->AddRaw(this, &SMain::OnStartPlay);
				OnStopEvent = &(Sequencer->OnStopEvent());
				OnStopEvent->AddRaw(this, &SMain::OnStopPlay);

				LoadMotionHandlersFromDisk(MotionHandlers);
				ListViewWidget->RequestListRefresh();
				IsSequencerRelevant = true;
				UE_LOG(LogTemp, Warning, TEXT("Sequencer refreshed"));
			};
		}
	}
}
void SMain::RefreshSequences()
{
	UWorld* world = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport)->World();
	ULevel* level = world->GetCurrentLevel();
	for (AActor* Actor : level->Actors)
	{
		if (Actor->GetClass()->IsChildOf(ALevelSequenceActor::StaticClass()))
		{
			ALevelSequenceActor* SequenceActor = Cast<ALevelSequenceActor>(Actor);
			SequenceActor->GetSequencePlayer();
			ULevelSequence* Sequence = SequenceActor->GetSequence();
			Sequences.Add(Sequence);
			break;
		};
	}
}
void SMain::ChangeSelectedSequence(ULevelSequence* Sequence_)
{
	if (Sequence_ != nullptr)
	{
		IsSequencerRelevant = false;
		SelectedSequence = Sequence_;
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
		/* UMovieSceneControlRigParameterTrack* controlRigTrack = Cast<UMovieSceneControlRigParameterTrack>(SelectedTracks[0]);
		if (IsValid(controlRigTrack))
		{
		  UE_LOG(LogTemp, Warning, TEXT("IT'S CONTROL rig TRACK!"));
		} */
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
					ListViewWidget->ClearSelection();
					ListViewWidget->SetItemSelection(handler, true, ESelectInfo::Type::Direct);
				}
			}
			if (!IsObjectAlreadyAdded)
			{
				MotionHandlers.Add(motionHandler);
				ListViewWidget->RequestListRefresh();
				ListViewWidget->ClearSelection();
				ListViewWidget->SetItemSelection(motionHandler, true, ESelectInfo::Type::Direct);
			}
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
		UE_LOG(LogTemp, Warning, TEXT("Trying to find files in  %s"), *SavesDir);
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
	if (false)	  // keep that code just to remember how to execute motion handlers every tick
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
	for (TSharedPtr<MotionHandler> motionHandler : MotionHandlers)
	{
		motionHandler->SetSelectedMode(Mode::X);
	}
	return FReply::Handled();
}
FReply SMain::SelectXInverted()
{
	for (TSharedPtr<MotionHandler> motionHandler : MotionHandlers)
	{
		motionHandler->SetSelectedMode(Mode::XInverted);
	}
	return FReply::Handled();
}
FReply SMain::SelectY()
{
	for (TSharedPtr<MotionHandler> motionHandler : MotionHandlers)
	{
		motionHandler->SetSelectedMode(Mode::Y);
	}
	return FReply::Handled();
}
FReply SMain::SelectYInverted()
{
	for (TSharedPtr<MotionHandler> motionHandler : MotionHandlers)
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
}
void SMain::OnKeyDownGlobal(const FKeyEvent& event)
{
	FKey key = event.GetKey();
	if (key.ToString() == "W")
	{
		RefreshSequencer();
		AddMotionHandlers();
	}
	if (key.ToString() == "E")
	{
		if (SelectedSequence != nullptr && Sequencer != nullptr)
		{
			IsRecordedStarted = true;

			TRange<FFrameNumber> playbackRange = SelectedSequence->GetMovieScene()->GetPlaybackRange();

			FFrameNumber lowerValue = playbackRange.GetLowerBoundValue();
			FFrameNumber highValue = playbackRange.GetUpperBoundValue();

			Sequencer->Pause();
			Sequencer->SetGlobalTime(lowerValue);

			PreviousPosition = FSlateApplication::Get().GetCursorPos();
			for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
			{
				motionHandler->PreviousValue = (double) motionHandler->GetValueFromTime(lowerValue);

				FFrameNumber DeleteKeysFrom = lowerValue;
				DeleteKeysFrom.Value += 3000;
				motionHandler->DeleteKeysWithin(TRange<FFrameNumber>(DeleteKeysFrom, highValue));
			}
			FMovieSceneSequencePlaybackParams params = FMovieSceneSequencePlaybackParams();
			params.Frame = highValue;
			Sequencer->PlayTo(params);
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
			for (TSharedPtr<MotionHandler> motionHandler : MotionHandlers)
			{
				motionHandler->Optimize(playbackRange);
				motionHandler->PreviousValue = (double) motionHandler->GetValueFromTime(lowerValue);
				motionHandler->SaveData();
			}
			Sequencer->Pause();
			Sequencer->SetGlobalTime(lowerValue);
			FMovieSceneSequencePlaybackParams params = FMovieSceneSequencePlaybackParams();
			params.Frame = highValue;
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
