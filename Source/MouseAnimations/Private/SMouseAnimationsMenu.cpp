// Fill out your copyright notice in the Description page of Project Settings.

#include "SMouseAnimationsMenu.h"

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSequenceActor.h"
#include "Math/Vector2D.h"
#include "SlateOptMacros.h"
#include "Widgets/SCompoundWidget.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMouseAnimationsMenu::Construct(const FArguments& InArgs)
{
	UWorld* world = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport)->World();
	ULevel* level = world->GetCurrentLevel();
	for (AActor* Actor : level->Actors)
	{
		if (Actor->GetClass()->IsChildOf(ALevelSequenceActor::StaticClass()))
		{
			ALevelSequenceActor* sequence = Cast<ALevelSequenceActor>(Actor);
			std::cout << "there's a sequence!";
		};
	}

	someContent = FText::FromString("old Content!");
	ChildSlot[SNew(SVerticalBox) +
			  SVerticalBox::Slot()
				  .AutoHeight()[SNew(SHorizontalBox) +
								SHorizontalBox::Slot().VAlign(VAlign_Top)[SNew(STextBlock).Text(FText::FromString("Test Button"))] +
								SHorizontalBox::Slot().VAlign(VAlign_Top)[SNew(SButton)
																			  .Text(this, &SMouseAnimationsMenu::GetSomeContent)
																			  .OnClicked(FOnClicked::CreateSP(this,
																				  &SMouseAnimationsMenu::OnTestButtonClicked))]] +
			  SVerticalBox::Slot().AutoHeight()
				  [SNew(SHorizontalBox) + SHorizontalBox::Slot().VAlign(VAlign_Top)[SNew(STextBlock).Text(InArgs._LabelText)] +
					  SHorizontalBox::Slot().VAlign(VAlign_Top)[SNew(SCheckBox).OnCheckStateChanged(
						  FOnCheckStateChanged::CreateSP(this, &SMouseAnimationsMenu::OnTestCheckboxStateChanged))]]];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SMouseAnimationsMenu::OnTestCheckboxStateChanged(ECheckBoxState NewState)
{
	bIsTestBoxChecked = NewState == ECheckBoxState::Checked ? true : false;
};
ECheckBoxState SMouseAnimationsMenu::IsTestBoxChecked() const
{
	return bIsTestBoxChecked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
};
FReply SMouseAnimationsMenu::OnTestButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Hello, world! The checkbox is %s."), (bIsTestBoxChecked ? TEXT("checked") : TEXT("unchecked")));
	someContent = FText::FromString("some new string!");
	return FReply::Handled();
};
FText SMouseAnimationsMenu::GetSomeContent() const
{
	std::cout << "get some content";
	return someContent;
};
FVector2D SMouseAnimationsMenu::GetCursorPosition() const
{
	std::cout << "GetCursor position";
	return FSlateApplication::Get().GetCursorPos();
};
void SMouseAnimationsMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	float desiredDeltaTime = 1 / fps;
	float timeFromLatest = (float) (InCurrentTime - timeWhenLastApplied);

	if (timeFromLatest >= desiredDeltaTime)
	{
		timeWhenLastApplied = InCurrentTime;
		std::cout << "pass";
		std::cout << timeFromLatest << std::endl;
	}
	else
	{
		std::cout << "skipped ";
		std::cout << timeFromLatest << std::endl;
	}
	/*std::cout<<InDeltaTime;
	std::cout<<"Tick fired!!!!";*/
}
