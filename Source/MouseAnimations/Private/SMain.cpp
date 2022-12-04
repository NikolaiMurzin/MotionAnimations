
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
#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "Framework/SlateDelegates.h"
#include "IKeyArea.h"
#include "ILevelSequenceEditorToolkit.h"
#include "ISequencer.h"
#include "Input/Reply.h"
#include "LevelSequence.h"
#include "Logging/LogMacros.h"
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
	SetSequence();
	SetSequencer();

	ChildSlot[SNew(SHorizontalBox) + SHorizontalBox::Slot()[SNew(SButton)
																.Text(FText::FromString("refresh sequence"))
																.OnClicked(FOnClicked::CreateSP(this, &SMain::OnButtonClicked))]];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

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
void SMain::SetSequencer()
{
	ISequencer* seq = GetSequencerFromSelectedSequence();
	if (seq != nullptr)
	{
		Sequencer = seq;
		OnGlobalTimeChangedDelegate = &(Sequencer->OnGlobalTimeChanged());
		OnGlobalTimeChangedDelegate->AddRaw(this, &SMain::OnGlobalTimeChanged);
	};
}

void SMain::OnGlobalTimeChanged(){
	UE_LOG(LogTemp, Warning, TEXT("OnGlobalTimeChanged!")) UE_LOG(LogTemp, Warning, TEXT("OnGlobalTimeChanged!"))};

ULevelSequence* SMain::GetSequence() const
{
	if (Sequence != nullptr)
	{
		return Sequence;
	}
	return GetLevelSequenceFromWorld();
}
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
void SMain::SetSequence()
{
	Sequence = GetLevelSequenceFromWorld();
}

FReply SMain::OnButtonClicked()
{
	SetSequence();
	SetSequencer();
	return FReply::Handled();
};

void SMain::SetChannelHandleFromSelectedKeys()
{
	if (Sequencer != nullptr)
	{
		TArray<const IKeyArea*> outKeys = TArray<const IKeyArea*>();
		Sequencer->GetSelectedKeyAreas(outKeys);
		FMovieSceneChannelHandle channelHandle = outKeys[0]->GetChannel();
		FName type = outKeys[0]->GetChannelTypeName();
		ChannelHandle = channelHandle;
		if (type.IsValid())
		{
			ChannelType = type;
		};
		if (type == "MovieSceneFloatChannel")
		{
			TMovieSceneChannelHandle<FMovieSceneFloatChannel> channelFloatHandle = channelHandle.Cast<FMovieSceneFloatChannel>();

			FMovieSceneFloatChannel* channel = channelFloatHandle.Get();
			UE_LOG(LogTemp, Warning, TEXT("It's float channel "));
			TArrayView<const FMovieSceneFloatValue> valueS = channel->GetValues();
			for (const FMovieSceneFloatValue value : valueS)
			{
				UE_LOG(LogTemp, Warning, TEXT("It's float value: %f"), value.Value);
			}
		}
		else if (type == "MovieSceneDoubleChannel")
		{
			TMovieSceneChannelHandle<FMovieSceneDoubleChannel> channelDoubleHandle = channelHandle.Cast<FMovieSceneDoubleChannel>();

			FMovieSceneDoubleChannel* channel = channelDoubleHandle.Get();
			UE_LOG(LogTemp, Warning, TEXT("It's double channel "));
		}
		else if (type == "MovieSceneBoolChannel")
		{
			TMovieSceneChannelHandle<FMovieSceneBoolChannel> channelBoolHandle = channelHandle.Cast<FMovieSceneBoolChannel>();

			FMovieSceneBoolChannel* channel = channelBoolHandle.Get();
			UE_LOG(LogTemp, Warning, TEXT("It's bool channel "));
		}
	}
}

