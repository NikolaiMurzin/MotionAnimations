
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
	RefreshSequence();

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
	RefreshSequencer();
	return FReply::Handled();
}

void SMain::RefreshSequencer()
{
	UAssetEditorSubsystem* UAssetEditorSubs = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	IAssetEditorInstance* AssetEditor = UAssetEditorSubs->FindEditorForAsset(Sequence, true);
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
		};
	}
}
void SMain::RefreshSequence()
{
	UWorld* world = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport)->World();
	ULevel* level = world->GetCurrentLevel();
	for (AActor* Actor : level->Actors)
	{
		if (Actor->GetClass()->IsChildOf(ALevelSequenceActor::StaticClass()))
		{
			ALevelSequenceActor* SequenceActor = Cast<ALevelSequenceActor>(Actor);
			SequenceActor->GetSequencePlayer();
			Sequence = SequenceActor->GetSequence();
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
	TArray<const IKeyArea*> KeyAreas = TArray<const IKeyArea*>();
	Sequencer->GetSelectedKeyAreas(KeyAreas);
	MotionHandlerPtrs = TSharedPtr<TArray<TSharedPtr<MotionHandler>>>(new TArray<TSharedPtr<MotionHandler>>());
	for (const IKeyArea* KeyArea : KeyAreas)
	{
		TSharedPtr<MotionHandler> motionHandler = TSharedPtr<MotionHandler>(new MotionHandler(KeyArea, 1, X));
		MotionHandlerPtrs->Add(motionHandler);
	}
}
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
void SMain::ExecuteMotionHandlers()
{
	FFrameNumber nextFrame = Sequencer->GetGlobalTime().Time.CeilToFrame();
	if (PreviousPosition.X == 0 && PreviousPosition.Y == 0)
	{
		if (MotionHandlerPtrs->Num() > 0)
		{
			for (TSharedPtr<MotionHandler> motionHandler : *MotionHandlerPtrs)
			{
				motionHandler->SetKey(nextFrame, FVector2D(0, 0));
			}
		}
	}
	else
	{
		FVector2D currentPosition = FSlateApplication::Get().GetCursorPos();
		FVector2D vectorChange = PreviousPosition - currentPosition;
		if (MotionHandlerPtrs->Num() > 0)
		{
			for (TSharedPtr<MotionHandler> motionHandler : *MotionHandlerPtrs)
			{
				motionHandler->SetKey(nextFrame, vectorChange);
			}
		}
	}
	PreviousPosition = FSlateApplication::Get().GetCursorPos();
}

