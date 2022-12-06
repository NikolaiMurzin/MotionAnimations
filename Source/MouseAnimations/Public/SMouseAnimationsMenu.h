// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelSequenceActor.h"
#include "Widgets/SCompoundWidget.h"

/**
 *
 */
class MOUSEANIMATIONS_API SMouseAnimationsMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMouseAnimationsMenu)
	{
	}
	SLATE_ATTRIBUTE(FText, LabelText)
	SLATE_END_ARGS()
	float SomeFloat;
	FReply OnTestButtonClicked();
	void OnTestCheckboxStateChanged(ECheckBoxState NewState);
	ECheckBoxState IsTestBoxChecked() const;

	FText GetSomeContent() const;
	FText someContent;

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	bool IsCaptureMode;
	float fps = 24;
	double timeWhenLastApplied;
	FVector2D GetCursorPosition() const;
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime);

	TArray<ALevelSequenceActor> SequenceActors;

protected:
	bool bIsTestBoxChecked;
};
