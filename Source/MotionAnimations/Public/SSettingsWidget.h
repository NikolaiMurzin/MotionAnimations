// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Settings.h"
#include "Widgets/SCompoundWidget.h"

/**
 *
 */
class MOTIONANIMATIONS_API SSettingsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSettingsWidget)
	{
	}
	SLATE_END_ARGS()

	SSettingsWidget();
	~SSettingsWidget();
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	void RebuildMainBox();
	TSharedPtr<SVerticalBox> MainBox;

	DECLARE_MULTICAST_DELEGATE(FOnChangeKeyEvent);
	FOnChangeKeyEvent* OnChangeKeyEvent = new FOnChangeKeyEvent();

	FSlateApplication::FOnApplicationPreInputKeyDownListener* OnKeyDownEvent;
	void OnKeyDownRaw(const FKeyEvent& event);

	FSettings* Settings;

	FText GetError() const;
	FString Error;

private:
	FString SelectedFunction;
	FButtonStyle* ClickedButtonStyle;
	FTextBlockStyle errStyle;
};
