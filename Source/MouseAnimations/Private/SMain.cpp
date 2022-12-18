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
#include "Misc/Paths.h"
#include "Misc/QualifiedFrameTime.h"
#include "MotionHandler.h"
#include "MotionHandlerData.h"
#include "MovieScene.h"
#include "MovieSceneBinding.h"
#include "MovieSceneSection.h"
#include "STreeViewSequencer.h"
#include "Sequencer/Public/SequencerAddKeyOperation.h"
#include "SequencerAddKeyOperation.h"
#include "SlateOptMacros.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Templates/SharedPointer.h"
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
	IsStarted = false;
	RefreshSequences();
	SelectedSequence = Sequences[0];
	RefreshSequencer();

	FSlateApplication& app = FSlateApplication::Get();
	OnKeyDownEvent = &(app.OnApplicationPreInputKeyDownListener());
	OnKeyDownEvent->AddRaw(this, &SMain::OnKeyDownGlobal);

	ChildSlot[SNew(SScrollBox) + SScrollBox::Slot()[SAssignNew(ListViewWidget, SListView<TSharedPtr<MotionHandler>>)
														.ItemHeight(24)
														.ListItemsSource(&MotionHandlers)
														.OnGenerateRow(this, &SMain::OnGenerateRowForList)]];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<ITableRow> SMain::OnGenerateRowForList(TSharedPtr<MotionHandler> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	FString trackName = Item->MovieSceneTrack->GetName();
	FString movieSceneName = Item->MovieScene->GetName();
	FString keyAreaName = Item->KeyArea->GetName().ToString();
	FMovieSceneChannelHandle channelHandle = Item->KeyArea->GetChannel();
	FString channelName = channelHandle.GetMetaData()->DisplayText.ToString();
	FString channelDisplayText = channelHandle.GetMetaData()->Name.ToString();
	FString group = channelHandle.GetMetaData()->Group.ToString();
	FString sortOrder = FString::FromInt(channelHandle.GetMetaData()->SortOrder);
	FString sectionName = Item->KeyArea->GetOwningSection()->GetName();
	FString rowIndexSection = FString::FromInt(Item->KeyArea->GetOwningSection()->GetRowIndex());
	return SNew(STableRow<TSharedPtr<MotionHandler>>, OwnerTable)
		.Padding(2.0f)
		.Content()[SNew(STextBlock)
					   .Text(FText::FromString(Item->GetObjectFGuid().ToString() + ", " + trackName + ", " + movieSceneName + ", " +
											   keyAreaName + ", " + channelName + ", " + channelDisplayText + ", " + group + ", " +
											   sortOrder + ", " + sectionName + ", " + rowIndexSection))];
}

FReply SMain::OnRefreshSequencer()
{
	RefreshSequencer();
	return FReply::Handled();
}

void SMain::RefreshSequencer()
{
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
				Sequencer = seq;
				OnGlobalTimeChangedDelegate = &(Sequencer->OnGlobalTimeChanged());
				OnGlobalTimeChangedDelegate->AddRaw(this, &SMain::OnGlobalTimeChanged);
				OnPlayEvent = &(Sequencer->OnPlayEvent());
				OnPlayEvent->AddRaw(this, &SMain::OnStartPlay);
				OnStopEvent = &(Sequencer->OnStopEvent());
				OnStopEvent->AddRaw(this, &SMain::OnStopPlay);
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
FReply SMain::OnRefreshBindings()
{
	RefreshMotionHandlers();
	return FReply::Handled();
};
void SMain::RefreshMotionHandlers()
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

		MotionHandlers = TArray<TSharedPtr<MotionHandler>>();
		ListViewWidget->SetListItemsSource(MotionHandlers);
		ListViewWidget->RequestListRefresh();
		for (const IKeyArea* KeyArea : KeyAreas)
		{
			TSharedPtr<MotionHandler> motionHandler = TSharedPtr<MotionHandler>(new MotionHandler(
				KeyArea, DefaultScale, Sequencer, SelectedSequence, SelectedTracks[0], SelectedObjects[0], Mode::X));

			FFrameNumber currentFrame = Sequencer->GetGlobalTime().Time.GetFrame();
			float value = motionHandler->GetValueFromTime(currentFrame);
			motionHandler->SetKey(Sequencer->GetGlobalTime().Time.GetFrame(),
				FVector2D(value, value));	 // need to add two keys for enabling recording motions
			FFrameNumber frame = Sequencer->GetGlobalTime().Time.GetFrame();
			frame.Value += 1000;
			motionHandler->SetKey(frame, FVector2D(value, value));

			MotionHandlers.Add(motionHandler);
		}
	}
}
FReply SMain::OnToggleTestAnimations()
{
	if (IsRecordedStarted)
	{
		return FReply::Handled();
	}
	IsTestAnimations = !IsTestAnimations;
	return FReply::Handled();
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
	if (IsTestAnimations)
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
	TArray<TSharedRef<IKeyArea>> keyAreas = TArray<TSharedRef<IKeyArea>>();
	if (PreviousPosition.X == 0 && PreviousPosition.Y == 0)
	{
		if (MotionHandlers.Num() > 0)
		{
			for (TSharedPtr<MotionHandler> motionHandler : MotionHandlers)
			{
				motionHandler->SetKey(nextFrame, FVector2D(0, 0));
			}
		}
	}
	else
	{
		FVector2D currentPosition = FSlateApplication::Get().GetCursorPos();
		FVector2D vectorChange = PreviousPosition - currentPosition;
		if (MotionHandlers.Num() > 0)
		{
			for (TSharedPtr<MotionHandler> motionHandler : MotionHandlers)
			{
				motionHandler->SetKey(nextFrame, vectorChange);
			}
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
	if (key.ToString() == "R")
	{
		OnToggleRecording();
	}
	if (key.ToString() == "T")
	{
		OnToggleTestAnimations();
	}
	if (key.ToString() == "Q")
	{
		RefreshSequencer();
	}
	if (key.ToString() == "W")
	{
		RefreshMotionHandlers();
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
			for (TSharedPtr<MotionHandler> motionHandler : MotionHandlers)
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
FReply SMain::OnToggleRecording()
{
	if (IsTestAnimations)
	{
		return FReply::Handled();
	}
	IsRecordedStarted = !IsRecordedStarted;
	PreviousPosition = FSlateApplication::Get().GetCursorPos(); /* get mouse current pos and set /./ &*/
	return FReply::Handled();
}
FText SMain::GetIsToggledRecording() const
{
	if (IsRecordedStarted)
	{
		return FText::FromString("Stop recording");
	}
	else
	{
		return FText::FromString("Start recording");
	}
}
