// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Channels/MovieSceneChannelProxy.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"

/**
 * 
 */
class MotionEditor
{
public:
	MotionEditor(FMovieSceneFloatChannel* floatChannel = nullptr, FMovieSceneDoubleChannel* doubleChannel = nullptr, FMovieSceneIntegerChannel* integerChannel = nullptr);
	~MotionEditor();
	void Edit(FFrameNumber InTime, double value);
	void ReInit();
	void Reset(TRange<FFrameNumber> range);
private:
	FMovieSceneFloatChannel* FloatChannel;
	FMovieSceneDoubleChannel* DoubleChannel;
	FMovieSceneIntegerChannel* IntegerChannel;

	FMovieSceneFloatChannel FloatChannelDup;
	FMovieSceneDoubleChannel DoubleChannelDup;
	FMovieSceneIntegerChannel IntegerChannelDup;

};