// Fill out your copyright notice in the Description page of Project Settings.

#include "SSettingsWidget.h"

#include "Brushes/SlateColorBrush.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SScrollBox.h"

SSettingsWidget::SSettingsWidget()
{
	Settings = new FSettings();
	SelectedFunction = "";
	Error = "";


	OnKeyDownEvent = &(FSlateApplication::Get().OnApplicationPreInputKeyDownListener());
	OnKeyDownEvent->AddRaw(this, &SSettingsWidget::OnKeyDown);

	// todo need to try to set it to button and then change that style
	ClickedButtonStyle = new FButtonStyle();
	ClickedButtonStyle->SetNormal(FSlateColorBrush(FLinearColor(1.f, 0.f, 0.f, 0.5f)));
	ClickedButtonStyle->SetHovered(FSlateColorBrush(FLinearColor(1.f, 0.f, 0.f, 0.75f)));
	ClickedButtonStyle->SetPressed(FSlateColorBrush(FLinearColor(1.f, 0.f, 0.f, 1.f)));

	errStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText");
	errStyle.ColorAndOpacity = FSlateColor(FLinearColor::Red);
}
SSettingsWidget::~SSettingsWidget()
{
	OnKeyDownEvent->RemoveAll(this);
	delete Settings;
}
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION void SSettingsWidget::Construct(const FArguments& InArgs)
{
	SAssignNew(MainBox, SVerticalBox);

	RebuildMainBox();


	ChildSlot
		[SNew(SScrollBox) +
			SScrollBox::Slot()[SNew(SHorizontalBox) +
							   SHorizontalBox::Slot()[SNew(STextBlock).TextStyle(&errStyle).Text(this, &SSettingsWidget::GetError)]
								   .HAlign(HAlign_Left) +
							   SHorizontalBox::Slot()[SNew(SButton)
														  .Content()[SNew(STextBlock).Text(FText::FromString("Reset"))]
														  .OnClicked_Lambda(
															  [&]()
															  {
																  Settings->SetDefaultSettings();
																  Settings->Save();
																  RebuildMainBox();
																  return FReply::Handled();
															  })
														  .DesiredSizeScale(TAttribute<FVector2D>(FVector2D(0.5, 2)))
														  .HAlign(HAlign_Right)]]

				 +
			SScrollBox::Slot()[MainBox.ToSharedRef()]];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSettingsWidget::RebuildMainBox()
{
	MainBox->ClearChildren();
	for (auto& Elem : Settings->Keys)
	{
		TSharedRef<SHorizontalBox> Block = SNew(SHorizontalBox);
		TSharedRef<STextBlock> Name = SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%s:"), *Elem.Key)));
		TSharedRef<STextBlock> Key = SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%s"), *Elem.Value)));
		TSharedRef<SButton> Button = SNew(SButton).Content()[SNew(STextBlock).Text(FText::FromString("Set"))].OnClicked_Lambda(
			[&]()
			{
				SelectedFunction = Elem.Key;
				return FReply::Handled();
			});
		Block->AddSlot().HAlign(HAlign_Left)[Name];
		Block->AddSlot().HAlign(HAlign_Center)[Key];
		Block->AddSlot().HAlign(HAlign_Right)[Button];

		// Add the text block to the vertical box
		MainBox->AddSlot().AutoHeight()[Block];
	}
}
void SSettingsWidget::OnKeyDown(const FKeyEvent& event)
{
	FString key = event.GetKey().ToString();
	if (SelectedFunction != "")
	{
		if (Settings->Keys.Contains(SelectedFunction))
		{
			for (auto& KeyValue : Settings->Keys)	 // if that key is already set
			{
				if (KeyValue.Value == key)
				{
					Error = "Key already set";
					return;
				}
			}
			Settings->Keys[SelectedFunction] = key;
			Settings->Save();
			this->RebuildMainBox();
			SelectedFunction = "";
			Error = "";
			OnChangeKeyEvent->Broadcast();
		}
	}
}
FText SSettingsWidget::GetError() const
{
	return FText::FromString(Error);
}
