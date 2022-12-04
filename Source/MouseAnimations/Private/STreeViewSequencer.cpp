// Fill out your copyright notice in the Description page of Project Settings.


#include "STreeViewSequencer.h"

#include "MovieSceneBinding.h"
#include "SlateOptMacros.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void STreeViewSequencer::Construct(const FArguments& InArgs)
{
  Sequence = InArgs._Sequence.Get();
  GetBindings();
	ChildSlot
	[
    SNew(SHorizontalBox)
      +SHorizontalBox::Slot()
      [
        SNew(STreeView<TSharedPtr<FMovieSceneBinding>>)
          .SelectionMode( ESelectionMode::Single )
          .ClearSelectionOnClick( false )
          .OnGenerateRow(this, &STreeViewSequencer::OnGenerateRowBindings)
          .TreeItemsSource(&Bindings)
          /*.OnGetChildren(this, &STreeViewSequencer::OnGetChildrenBindings)*/
      ]
      +SHorizontalBox::Slot()
      [
        SNew(SButton)
          .Text(FText::FromString("refresh data"))
          .OnClicked(FOnClicked::CreateSP(this, &STreeViewSequencer::OnButtonClicked))
      ]
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<ITableRow> STreeViewSequencer::OnGenerateRowBindings ( TSharedPtr<FMovieSceneBinding> Item, const TSharedRef<STableViewBase>& OwnerTable )
{
    std::cout<<"OnGenerateRowBindings called";
		return SNew( STableRow< TSharedPtr<FMovieSceneBinding>>, OwnerTable )
		[
			SNew(STextBlock)
			.Text( FText::FromString("Hello world!") )
		];
}
/*void STreeViewSequencer::OnGetChildrenBindings(TSharedPtr<FMovieSceneBinding > Item, TArray<TSharedPtr<FMovieSceneBinding>>& OutChildren )
{
  const  Tracks = Item->GetTracks(); 
	OutChildren.Append( Tracks );
  std::cout<<"OngetChildren Called";
}*/
TArray<TSharedPtr<FMovieSceneBinding>>* STreeViewSequencer::GetBindings()
{
  TArray<TSharedPtr<FMovieSceneBinding>> result = TArray<TSharedPtr<FMovieSceneBinding>>();
  for (FMovieSceneBinding b : Sequence->GetMovieScene()->GetBindings())
  {
    result.Add(TSharedPtr<FMovieSceneBinding>(&b));
  }
  Bindings = result;
  return &Bindings;
}

FReply STreeViewSequencer::OnButtonClicked()
{
  GetBindings();
	return FReply::Handled();
};
