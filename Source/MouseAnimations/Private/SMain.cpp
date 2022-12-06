
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
#include "Framework/SlateDelegates.h"
#include "ILevelSequenceEditorToolkit.h"
#include "Input/Reply.h"
#include "LevelSequence.h"
#include "Logging/LogMacros.h"
#include "Misc/QualifiedFrameTime.h"
#include "MotionHandler.h"
#include "MovieScene.h"
#include "MovieSceneBinding.h"
#include "STreeViewSequencer.h"
#include "SlateOptMacros.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Templates/SharedPointer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <typeinfo>

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMain::Construct(const FArguments& InArgs)
{
	DefaultScale = 0.10;

	ChildSlot[SNew(SHorizontalBox) +
			  SHorizontalBox::Slot()[SNew(SButton)
										 .Text(FText::FromString("refresh Sequencer"))
										 .OnClicked(FOnClicked::CreateSP(this, &SMain::OnRefreshSequencer))] +
			  SHorizontalBox::Slot()[SNew(SButton)
										 .Text(FText::FromString("refresh Bindings"))
										 .OnClicked(FOnClicked::CreateSP(this, &SMain::OnRefreshBindings))] +
			  SHorizontalBox::Slot()[SNew(SButton)
										 .Text(FText::FromString("start / stop recording"))
										 .OnClicked(FOnClicked::CreateSP(this, &SMain::OnToggleRecording))] +
			  SHorizontalBox::Slot()[SNew(SButton)
										 .Text(FText::FromString("start / stop test animations"))
										 .OnClicked(FOnClicked::CreateSP(this, &SMain::OnToggleTestAnimations))]];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SMain::OnRefreshSequencer()
{
	Sequence = GetLevelSequenceFromWorld();
	RefreshSequencer();
	return FReply::Handled();
}

void SMain::RefreshSequencer()
{
	ISequencer* seq = GetSequencerFromSelectedSequence();
	if (seq != nullptr)
	{
		Sequencer = seq;
		OnGlobalTimeChangedDelegate = &(Sequencer->OnGlobalTimeChanged());
		OnGlobalTimeChangedDelegate->AddRaw(this, &SMain::OnGlobalTimeChanged);
	};
}
ISequencer* SMain::GetSequencerFromSelectedSequence()
{
	UAssetEditorSubsystem* UAssetEditorSubs = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	IAssetEditorInstance* AssetEditor = UAssetEditorSubs->FindEditorForAsset(Sequence, true);
	ILevelSequenceEditorToolkit* LevelSequenceEditor = (ILevelSequenceEditorToolkit*) AssetEditor;
	if (LevelSequenceEditor != nullptr)
	{
		// Get current Sequencer
		ISequencer* seq = LevelSequenceEditor->GetSequencer().Get();
		return seq;
	}
	return nullptr;
};
ISequencer* SMain::GetSequencer()
{
	return Sequencer;
};
ULevelSequence* SMain::GetLevelSequenceFromWorld() const
{
	UWorld* world = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport)->World();
	ULevel* level = world->GetCurrentLevel();
	for (AActor* Actor : level->Actors)
	{
		if (Actor->GetClass()->IsChildOf(ALevelSequenceActor::StaticClass()))
		{
			ALevelSequenceActor* SequenceActor = Cast<ALevelSequenceActor>(Actor);
			SequenceActor->GetSequencePlayer();
			ULevelSequence* sequence = SequenceActor->GetSequence();
			return sequence;
		};
	}
	throw std::logic_error{"Didn't find any sequence!"};
}

FReply SMain::OnRefreshBindings()
{
	RefreshMotionHandlers();
	return FReply::Handled();
};
FReply SMain::OnToggleRecording()
{
	if (IsTestAnimations)
	{
		return FReply::Handled();
	}
	IsRecordedStarted = !IsRecordedStarted;
	return FReply::Handled();
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
	UE_LOG(LogTemp, Warning, TEXT("OnGlobalTimeChanged!"));
	if (IsRecordedStarted)
	{
		ExecuteMotionHandlers();
	}
};
void SMain::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (IsTestAnimations)
	{
		float desiredDeltaTime = 1 / fps;
		float timeFromLatest = (float) (InCurrentTime - TimeFromLatestTestExecution);
		FVector2D cursorPos = FSlateApplication::Get().GetCursorPos();

		if (timeFromLatest >= desiredDeltaTime)
		{
			std::cout << "pass";
			ExecuteMotionHandlers();
			TimeFromLatestTestExecution = InCurrentTime;
		}
		else
		{
			std::cout << "skipped ";
		}
	}
}
void SMain::RefreshMotionHandlers()
{
	TArray<const IKeyArea*> KeyAreas = TArray<const IKeyArea*>();
	Sequencer->GetSelectedKeyAreas(KeyAreas);
	MotionHandlerPtrs = TArray<TSharedPtr<MotionHandler>>();
	for (const IKeyArea* KeyArea : KeyAreas)
	{
		TSharedPtr<MotionHandler> motionHandler = TSharedPtr<MotionHandler>(new MotionHandler(KeyArea, 1, X));
		MotionHandlerPtrs.Add(motionHandler);
	}
}
void SMain::ExecuteMotionHandlers()
{
	if (PreviousPosition.X == 0 && PreviousPosition.Y == 0)
	{
		FFrameNumber currentFrame = Sequencer->GetGlobalTime().Time.GetFrame();
		for (TSharedPtr<MotionHandler> motionHandler : MotionHandlerPtrs)
		{
			motionHandler->SetKey(currentFrame, FVector2D(0, 0));
		}
	}
	else
	{
		FFrameNumber currentFrame = Sequencer->GetGlobalTime().Time.GetFrame();
		FVector2D currentPosition = FSlateApplication::Get().GetCursorPos();
		FVector2D vectorChange = PreviousPosition - currentPosition;
		for (TSharedPtr<MotionHandler> motionHandler : MotionHandlerPtrs)
		{
			motionHandler->SetKey(currentFrame, vectorChange);
		}
	}
	PreviousPosition = FSlateApplication::Get().GetCursorPos();
	Sequencer->ForceEvaluate();
}

