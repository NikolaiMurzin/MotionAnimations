// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelSequence.h"
#include "MovieSceneBinding.h"
#include "MovieSceneTrack.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/ITableRow.h"
#include "Widgets/Views/STableViewBase.h"

/**
 * 
 */
class MOUSEANIMATIONS_API STreeViewSequencer : public SCompoundWidget
{
  public:
    SLATE_BEGIN_ARGS(STreeViewSequencer)
    {}
       SLATE_ATTRIBUTE(class ULevelSequence*, Sequence)
    SLATE_END_ARGS()

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs);
    ULevelSequence* Sequence;
    TArray<TSharedPtr<FMovieSceneBinding>>* GetBindings();
    TSharedRef<ITableRow> OnGenerateRowBindings(TSharedPtr<FMovieSceneBinding> Binding, const TSharedRef<STableViewBase>& OwnerTable);
    void OnGetChildrenBindings(TSharedPtr<FMovieSceneBinding> Item, TArray<TSharedPtr<FMovieSceneBinding>>& OutChildren );
    TArray<TSharedPtr<FMovieSceneBinding>> Bindings;
    FReply OnButtonClicked();
};
