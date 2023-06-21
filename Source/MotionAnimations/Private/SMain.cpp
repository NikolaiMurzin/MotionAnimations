// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

#include "SMain.h"
#include "InputCoreTypes.h"

#include "Components/InputComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetRegistryState.h"
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

#include "ISequencerModule.h"

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
	SelectedSectionsChangedEvent = nullptr;
	Sequencer = nullptr;
	SelectedSequence = nullptr;

	UAssetEditorSubsystem* UAssetEditorSubs = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	OpenEditorEvent = &(UAssetEditorSubs->OnAssetEditorOpened());
	OpenEditorEvent->AddRaw(this, &SMain::OnEditorOpened);

	MotionHandlers = TArray<TSharedPtr<MotionHandler>>();
	CustomRange = TRange<FFrameNumber>();
	CustomRange.SetUpperBound(FFrameNumber());
	CustomRange.SetLowerBound(FFrameNumber());

	FSlateApplication& app = FSlateApplication::Get();
	OnKeyDownEvent = &(app.OnApplicationPreInputKeyDownListener());
	OnKeyDownEvent->AddRaw(this, &SMain::OnKeyDownGlobal);

}
SMain::~SMain()
{
	if (SettingsWindow.IsValid())
	{
		SettingsWindow.Get()->DestroyWindowImmediately();
	}
	delete Settings;
	if (OpenEditorEvent != nullptr)
	{
		OpenEditorEvent->RemoveAll(this);
	}
	if (OnKeyDownEvent != nullptr)
	{
		OnKeyDownEvent->RemoveAll(this);
	}
	UnbindSequencerEvents();
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
		.OnValueChanged(this, &SMain::OnToleranceChangeRaw)]
		]
	.FillWidth(0.3f)
		.Padding(FMargin(0.f, 0.f, 10.f, 0.0f))
		.HAlign(HAlign_Right)
		+
		SHorizontalBox::Slot()[SNew(SHorizontalBox) +
		SHorizontalBox::Slot()[SNew(STextBlock)
		.Text(FText::FromString("Populate value"))]
		.Padding(FMargin(0.0f, 2.5f, 2.5f, 0.0f))
		.AutoWidth() +
		SHorizontalBox::Slot()[SNew(SSpinBox<double>)
		.Value(this->PopulateValue)
		.OnValueChanged(this, &SMain::OnPopulateValueChangedRaw)
		]
		]
		.FillWidth(0.3f)
		.Padding(FMargin(0.f, 0.f, 10.f, 0.0f))
		.HAlign(HAlign_Right)
		]
		.AutoHeight() +
		SVerticalBox::Slot()[SNew(STextBlock).Text(this, &SMain::GetCustomStartFromFrame)] +
		SVerticalBox::Slot()[SNew(STextBlock).Text(this, &SMain::GetCustomEndFrame)]]
	.Padding(FMargin(10.f, 0.f, 0.f, 5.f)) +
		SScrollBox::Slot()[SNew(SHorizontalBox) + SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Name"))] +
		SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Scale"))] +
		SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Current index"))] +
		SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Selected Mode"))] +
		SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Lower bound value"))] +
		SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Upper bound value"))]
		+ SHorizontalBox::Slot()[SNew(STextBlock).Text(FText::FromString("Set"))]
		]
		.Padding(FMargin(10.f, 5.f, 0.f, 5.f))
		+
		SScrollBox::Slot()[SAssignNew(ListViewWidget, SListView<TSharedPtr<MotionHandler>>)
		.ItemHeight(24)
		.ListItemsSource(&MotionHandlers)
		.OnGenerateRow(this, &SMain::OnGenerateRowForList)]];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

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
		[SNew(SSpinBox<double>).Value(Item->Data.UpperBoundValue).OnValueChanged(Item->OnUpperBoundValueChanged)] +
		SHorizontalBox::Slot()
		[SNew(SSpinBox<int32>).Value(Item->Data.SetIndex).OnValueChanged(Item->OnSetIndexChanged)]
		];
}

FText SMain::GetIsActive() const
{
	if (IsKeysEnabled)
	{
		return FText::FromString("Active");
	}
	return FText::FromString("Not active");
};

FReply SMain::OpenSettingsWindow()
{
	// if we already have window
	if (SettingsWindow.IsValid())
	{
		SettingsWindow->BringToFront();
		return FReply::Handled();
	}
	TSharedRef<SSettingsWidget> SettingsWidget = SNew(SSettingsWidget);
	SettingsWidget.Get().OnChangeKeyEvent->AddRaw(this, &SMain::RefreshSettings);

	SettingsWindow = SNew(SWindow).Title(FText::FromString(TEXT("Settings"))).ClientSize(FVector2D(400, 500));
	SettingsWindow->SetContent(SettingsWidget);
	SettingsWindow = FSlateApplication::Get().AddWindow(SettingsWindow.ToSharedRef());
	SettingsWindow->SetOnWindowClosed(FOnWindowClosed::CreateLambda([&](const TSharedRef<SWindow>& ClosedWindow)
		{
			SettingsWindow = nullptr;
		}));

	return FReply::Handled();
}
void SMain::RefreshSettings()
{
	Settings->LoadSettingsFromDisk();
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
	if (!SelectedSequence)
	{
		return TRange<FFrameNumber>();
	}
	if (IsCustomRange)
	{
		FFrameNumber lower = CustomRange.GetLowerBoundValue();
		FFrameNumber upper = CustomRange.GetUpperBoundValue();
		if (lower >= upper)
		{
			upper = SelectedSequence->GetMovieScene()->GetPlaybackRange().GetUpperBoundValue();
		}
		return TRange<FFrameNumber>(lower, upper);
	}
	else
	{
		return SelectedSequence->GetMovieScene()->GetPlaybackRange();
	}
	return TRange<FFrameNumber>();
}
FText SMain::GetCustomStartFromFrame() const
{
	if (IsCustomRange)
	{
		if (Sequencer != nullptr)
		{
			const FFrameRate rate = Sequencer->GetFocusedDisplayRate();
			const FFrameNumber frame = CustomRange.GetLowerBoundValue();
			float val = (rate.AsDecimal() / 24) * frame.Value;
			val /= 1000;

			FString value = FString::FromInt(int(val));
			FString string = FString("Start frame: ");
			string.Append(value);
			return FText::FromString(string);
		}
	}
	return FText::FromString("Custom start frame:");
}
FText SMain::GetCustomEndFrame() const
{
	if (IsCustomRange)
	{
		if (Sequencer != nullptr)
		{
			FString string = FString("End frame: ");

			const FFrameRate rate = Sequencer->GetFocusedDisplayRate();
			const FFrameNumber frame = CustomRange.GetUpperBoundValue();

			float val = (rate.AsDecimal() / 24) * frame.Value;
			val /= 1000;

			FString value = FString::FromInt(int(val));
			string.Append(value);
			return FText::FromString(string);
		}
	}
	return FText::FromString("Custom end frame:");
}
void SMain::SelectedSectionsChangedRaw(TArray<UMovieSceneSection*> sections)
{
	AddMotionHandlers();
	for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
	{
		TRange<FFrameNumber> range = GetCurrentRange();
		motionHandler->ReInitAccelerator(range);
		motionHandler->ReInitMotionEditor();
	}
}

void SMain::OnEditorOpened(UObject* object)
{
	RefreshSequence();
	RefreshSequencer();
}

void SMain::UnbindSequencerEvents()
{
	if (Sequencer != nullptr)
	{
		if (OnGlobalTimeChangedDelegate != nullptr)
		{
			OnGlobalTimeChangedDelegate->RemoveAll(this);
		}
		if (OnPlayEvent != nullptr)
		{
			OnPlayEvent->RemoveAll(this);
		}
		if (OnStopEvent != nullptr)
		{
			OnStopEvent->RemoveAll(this);
		}
		if (OnCloseEvent != nullptr)
		{
			OnCloseEvent->RemoveAll(this);
		}
		if (SelectedSectionsChangedEvent != nullptr)
		{
			SelectedSectionsChangedEvent->RemoveAll(this);
		}
	}
}

void SMain::RefreshSequence()
{
	UAssetEditorSubsystem* UAssetEditorSubs = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();

	TArray<UObject*> AssetsOne = UAssetEditorSubs->GetAllEditedAssets();

	for (UObject* AssetData : AssetsOne)
	{
		ULevelSequence* Sequence = Cast<ULevelSequence>(AssetData);
		if (Sequence)
		{
			SelectedSequence = Sequence;
		}
	}
}
void SMain::RefreshSequencer()
{
	if (SelectedSequence != nullptr)
	{
		Sequencer = nullptr;

		UAssetEditorSubsystem* UAssetEditorSubs = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		IAssetEditorInstance* AssetEditor = UAssetEditorSubs->FindEditorForAsset(SelectedSequence, false);

		TArray<UObject*> Assets = UAssetEditorSubs->GetAllEditedAssets();

		ILevelSequenceEditorToolkit* LevelSequenceEditor = (ILevelSequenceEditorToolkit*)AssetEditor;
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

				SelectedSectionsChangedEvent = &(Sequencer->GetSelectionChangedSections());
				SelectedSectionsChangedEvent->AddRaw(this, &SMain::SelectedSectionsChangedRaw);

				LoadMotionHandlersFromDisk(MotionHandlers);
				ListViewWidget->RequestListRefresh();
			};
		}
	}
}
void SMain::OnCloseEventRaw(TSharedRef<ISequencer> Sequencer_)
{
	UnbindSequencerEvents();
	SelectedSequence = nullptr;
	Sequencer = nullptr;
	MotionHandlers.Reset();
}
void SMain::AddMotionHandlers()
{
	if (Sequencer != nullptr && SelectedSequence != nullptr)
	{
		TArray<const IKeyArea*> KeyAreas = TArray<const IKeyArea*>();
		Sequencer->GetSelectedKeyAreas(KeyAreas);
		TArray<UMovieSceneTrack*> SelectedTracks = TArray<UMovieSceneTrack*>();
		Sequencer->GetSelectedTracks(SelectedTracks);
		if (SelectedTracks.Num() < 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("You are not selected any tracks or objects"));
			return;
		}
		TArray<TSharedPtr<MotionHandler>> selectionSet;
		for (const IKeyArea* KeyArea : KeyAreas)
		{
			bool IsObjectAlreadyAdded = false;

			UMovieSceneSection* section = KeyArea->GetOwningSection();
			auto forWhichTrackThatSection = [&]() -> UMovieSceneTrack* {
				for (UMovieSceneTrack* Track : SelectedTracks)
				{
					TArray<UMovieSceneSection*> Sections = Track->GetAllSections();
					if (Sections.Contains(section))
					{
						return Track;
						break;
					}
				}
				return nullptr;
			};
			UMovieSceneTrack*  trackOfThatSection = forWhichTrackThatSection();
			if ( trackOfThatSection )
			{
				FGuid objectGuid = trackOfThatSection ->FindObjectBindingGuid();

				MotionHandler newMotionHandler = MotionHandler(
					KeyArea, DefaultScale, Sequencer, SelectedSequence, trackOfThatSection , objectGuid, Mode::X);

				for (TSharedPtr<MotionHandler> alreadyAddedHandler : MotionHandlers)
				{
					if (*alreadyAddedHandler == newMotionHandler)
					{
						IsObjectAlreadyAdded = true;
						selectionSet.Add(alreadyAddedHandler);
					}
				}

				if (!IsObjectAlreadyAdded)
				{
					TSharedPtr<MotionHandler> newMotionHandlerPtr = TSharedPtr<MotionHandler>(new MotionHandler(
						KeyArea, DefaultScale, Sequencer, SelectedSequence, trackOfThatSection, objectGuid, Mode::X));
					MotionHandlers.Add(newMotionHandlerPtr);
					selectionSet.Add(newMotionHandlerPtr);
				}
			}
		}
		ListViewWidget->ClearSelection();
		ListViewWidget->SetItemSelection(selectionSet, true, ESelectInfo::Type::Direct);
		ListViewWidget->RequestListRefresh();
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
	FFrameNumber CurrentFrame = Sequencer->GetGlobalTime().Time.FrameNumber;
	if (IsRecordedStarted)
	{
		if (IsCustomRange)
		{
			FFrameNumber valueFromWhichStart = GetCurrentRange().GetLowerBoundValue().Value;
			valueFromWhichStart -= 1000; // we should start from -1000 of CustomRange because when we execute Motion Handlers we make +1000 FFrameNumber;
			if (CurrentFrame.Value >= valueFromWhichStart &&
				CurrentFrame.Value <= GetCurrentRange().GetUpperBoundValue().Value)
			{
				ExecuteMotionHandlers(vectorChange, CurrentFrame);
			}
		}
		else
		{
			ExecuteMotionHandlers(vectorChange, CurrentFrame);
		}
		Sequencer->RequestEvaluate();
	}
	if (IsScalingStarted)
	{
		if (MotionHandlers.Num() > 0)
		{
			CurrentFrame.Value += 1000;
			for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
			{
				motionHandler->Accelerate(vectorChange, CurrentFrame);
			}
		}
	}
	if (IsEditStarted)
	{
		if (IsCustomRange)
		{
			FFrameNumber CurrentTime = Sequencer->GetLocalTime().Time.FrameNumber;
			if (CurrentTime.Value >= GetCurrentRange().GetLowerBoundValue().Value && CurrentTime.Value <= GetCurrentRange().GetUpperBoundValue().Value)
			{
				FFrameNumber nextFrame = Sequencer->GetGlobalTime().Time.GetFrame();
				nextFrame.Value += 1000;
				for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
				{
					motionHandler->EditPosition(nextFrame, vectorChange);
				}
			}
		}
		else
		{
			FFrameNumber nextFrame = Sequencer->GetGlobalTime().Time.GetFrame();
			nextFrame.Value += 1000;
			for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
			{
				motionHandler->EditPosition(nextFrame, vectorChange);
			}
		}
	}
	PreviousPosition = FSlateApplication::Get().GetCursorPos();
};
void SMain::ExecuteMotionHandlers(FVector2D value, FFrameNumber frame)
{
	frame.Value += 1000;
	if (MotionHandlers.Num() > 0)
	{
		for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
		{
			FFrameNumber deleteFrom = frame;
			deleteFrom.Value += 1;
			FFrameNumber deleteTo = frame;
			deleteTo.Value += 3000;
			FFrameNumber upperRange = GetCurrentRange().GetUpperBoundValue();
			if (deleteTo >= upperRange)
			{
				deleteTo = upperRange;
			}
			motionHandler->DeleteKeysWithin(TRange<FFrameNumber>(deleteFrom, deleteTo)); // need to delete values that goes after current key on 5 seconds, so we will clean all frames continuously, not by once like before.

				// Sequencer keep update if there are keys in range 3000 from current time, if there are no keys, then it will freeze, and we won't see any changes when move our mouse.
			motionHandler->SetKey(frame, value);
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
	if (IsRecordedStarted)
	{
		FFrameNumber deleteFrom = Sequencer->GetGlobalTime().Time.FrameNumber;
		FFrameNumber deleteTo = GetCurrentRange().GetUpperBoundValue();
		for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
		{
			motionHandler->DeleteKeysWithin(TRange<FFrameNumber>(deleteFrom, deleteTo));
		}
	}

	IsStarted = false;
	IsRecordedStarted = false;
	IsScalingStarted = false;
	IsEditStarted = false;
}
void SMain::OnKeyDownGlobal(const FKeyEvent& event)
{
	// if no current opened sequencer
	if (Sequencer == nullptr)
	{
		return;
	}
	// if we have opened SettingsWindow
	if (SettingsWindow.IsValid() && SettingsWindow->IsActive())
	{
		return;
	}


	// check if it's focused not in main window or it's focused on Editable text
	TSharedPtr<SWidget> CurrentFocusedWidget = FSlateApplication::Get().GetCursorUser()->GetFocusedWidget();
	if (CurrentFocusedWidget.IsValid())
	{
		FString type = CurrentFocusedWidget->GetTypeAsString();

		TSharedPtr<SWindow> SelectedWindow = FSlateApplication::Get().FindWidgetWindow(CurrentFocusedWidget.ToSharedRef());
		if (SelectedWindow.IsValid())
		{
			if (SelectedWindow->GetParentWindow() != nullptr)	 // if it isn't main window
			{
				return;
			}
		}

		if (CurrentFocusedWidget != nullptr && type == "SEditableText") // if user now focused on Editable text;
		{
			return;
		}
		if (CurrentFocusedWidget != nullptr && type == "SViewport")
		{
			return;
		}
	}
	FString key = event.GetKey().ToString();
	if (Settings->Keys.Num() < 14)	  // there should be at least 14 settings
	{
		UE_LOG(LogTemp, Warning, TEXT("Something wrong with your settings file! Try to remove it from {ProjectDir}/{PluginDir}/MotionAnimations/Settings/ and start again, it will re-generate it"));
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
	if (Settings->Keys["Update motion handlers"] == key)
	{
		// reInit accelerator for selected motion handlers
		for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
		{
			TRange<FFrameNumber> range = GetCurrentRange();
			motionHandler->ReInitAccelerator(range);
			// motionHandler->ReInitMotionEditor();
		}
		// remove motion handlers that are empty
		TArray<TSharedPtr<MotionHandler>> toRemove;
		for (TSharedPtr<MotionHandler> motionHandler : MotionHandlers)
		{
			if (!motionHandler->HasData())
			{
				toRemove.Add(motionHandler);
			}
		}
		for (TSharedPtr<MotionHandler> motionHandler : toRemove)
		{
			motionHandler->DeleteData();
			MotionHandlers.Remove(motionHandler);
		}
	}
	TRange<FFrameNumber> playbackRange;
	FFrameNumber lowerValue;
	FFrameNumber highValue;
	if (SelectedSequence != nullptr)
	{
		playbackRange = SelectedSequence->GetMovieScene()->GetPlaybackRange();
		lowerValue = playbackRange.GetLowerBoundValue();
		highValue = playbackRange.GetUpperBoundValue();
	}
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
	if (Settings->Keys["Populate"] == key)
	{
		for (TSharedPtr<MotionHandler> handler : ListViewWidget->GetSelectedItems())
		{
			TRange<FFrameNumber> range = GetCurrentRange();
			FFrameNumber lower = range.GetLowerBoundValue();
			range.SetLowerBoundValue(lower);
			FFrameNumber interval;
			interval.Value = PopulateValue;
			handler->Populate(range, interval);
		}
		IsRecordedStarted = false;
		IsScalingStarted = false;
		IsEditStarted = false;
	}
	if (Settings->Keys["Start recording"] == key)
	{
		if (SelectedSequence != nullptr && Sequencer != nullptr)
		{
			LatestSyncTime = FFrameNumber(0);

			for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
			{
				FFrameNumber deleteFrom = GetCurrentRange().GetLowerBoundValue();
				if (IsCustomRange)
				{
					deleteFrom.Value += 1;
				}
				FFrameNumber deleteTo = GetCurrentRange().GetUpperBoundValue();
				deleteTo.Value += 1000;
				motionHandler->DeleteKeysWithin(TRange<FFrameNumber>(deleteFrom, deleteTo));
			}

			stopSequencerAndBackToFirstFrame();

			TRange<FFrameNumber> CurrentRange_ = GetCurrentRange();
			FFrameNumber lowerCurrentValue = CurrentRange_.GetLowerBoundValue();
			FFrameNumber upperValue = CurrentRange_.GetUpperBoundValue();

			for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
			{
				motionHandler->PreviousValue = (double)motionHandler->GetValueFromTime(lowerCurrentValue);

				motionHandler->Populate(TRange<FFrameNumber>(lowerCurrentValue, upperValue), FFrameNumber(1000)); // we need to populate whole Current range so sequencer won't freeze and will keep update
				// Sequencer keep update if there are keys in range 3000 from current time, if there are no keys, then it will freeze, and we won't see any changes when move our mouse.

			}

			playSequencerToLastFrame();

			IsScalingStarted = false;
			IsEditStarted = false;
			IsRecordedStarted = true;


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
	/* if (Settings->Keys["Start scaling"] == key)
	{
		stopSequencerAndBackToFirstFrame();

		TRange<FFrameNumber> CurrentRange_ = GetCurrentRange();

		for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
		{
			motionHandler->PreviousValue = (double)motionHandler->GetValueFromTime(CurrentRange_.GetLowerBoundValue());
			motionHandler->ResetAccelerator(CurrentRange_);
			motionHandler->ResetNiagaraState();
		}

		playSequencerToLastFrame();

		IsScalingStarted = true;
		IsEditStarted = false;
		IsRecordedStarted = false;
		PreviousPosition = FSlateApplication::Get().GetCursorPos();

		Sequencer->UpdatePlaybackRange();
		Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::RefreshAllImmediately);
		Sequencer->NotifyBindingsChanged();
	} */
	if (Settings->Keys["Stop recording"] == key)
	{
		if (SelectedSequence != nullptr && Sequencer != nullptr)
		{
			TRange<FFrameNumber> CurrentRange_ = GetCurrentRange();
			FFrameNumber lowerCurrentValue = CurrentRange_.GetLowerBoundValue();
			FFrameNumber upperValue = CurrentRange_.GetUpperBoundValue();


			stopSequencerAndBackToFirstFrame();
			for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
			{
				motionHandler->PreviousValue = (double)motionHandler->GetValueFromTime(lowerCurrentValue);

			}

		}
		else
		{
			UE_LOG(LogTemp, Warning,
				TEXT("No sequence selected! Or you selected wrong sequence, not the one that is open in sequencer"));
		}
	}
	if (Settings->Keys["Optimize"] == key)
	{
		if (SelectedSequence != nullptr && Sequencer != nullptr)
		{
			TRange<FFrameNumber> CurrentRange_ = GetCurrentRange();

			for (TSharedPtr<MotionHandler> motionHandler : ListViewWidget->GetSelectedItems())
			{
				motionHandler->Optimize(CurrentRange_, OptimizationTolerance);
			}
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
	else if (Settings->Keys["Activate custom range"] == key)
	{
		IsCustomRange = !IsCustomRange;
	}
	auto refreshSelection = [&](int setNumber)
	{
		if (Sequencer == nullptr)
		{
			return;
		}
		Sequencer->EmptySelection();
		for (TSharedPtr<MotionHandler> mh : MotionHandlers)
		{
			if (mh->Data.SetIndex == setNumber)
			{
				TArray<FMovieSceneChannelHandle> channels;
				FMovieSceneChannelHandle handle = mh->GetActualChannelHandle();
				channels.Add(handle);
				TArrayView<const FMovieSceneChannelHandle> channelsView = TArrayView<const FMovieSceneChannelHandle>(channels);
				Sequencer->SelectByChannels(mh->MovieSceneSection, channelsView, false, true);
			}
		}
	};
	if (Settings->Keys["select 1 set"] == key)
	{
		refreshSelection(1);
	}
	else if (Settings->Keys["select 2 set"] == key)
	{
		refreshSelection(2);
	}
	else if (Settings->Keys["select 3 set"] == key)
	{
		refreshSelection(3);
	}
	else if (Settings->Keys["select 4 set"] == key)
	{
		refreshSelection(4);
	}
	else if (Settings->Keys["select 5 set"] == key)
	{
		refreshSelection(5);
	}
	else if (Settings->Keys["select 6 set"] == key)
	{
		refreshSelection(6);
	}
	else if (Settings->Keys["select 7 set"] == key)
	{
		refreshSelection(7);
	}
	else if (Settings->Keys["select 8 set"] == key)
	{
		refreshSelection(8);
	}
	else if (Settings->Keys["select 9 set"] == key)
	{
		refreshSelection(9);
	}
}
void SMain::OnToleranceChangeRaw(double value)
{
	OptimizationTolerance = value;
}
void SMain::OnPopulateValueChangedRaw(double val)
{
	PopulateValue = val;
}
