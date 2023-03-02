// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Styling/SlateStyle.h"

/**  */
class FMotionAnimationsStyle
{
public:


  static void Initialize();


	static void Shutdown();

	/** reloads textures used by slate renderer */
	static void ReloadTextures();

	/** @return The Slate style set for the Shooter game */
	static const ISlateStyle& Get();

	static FName GetStyleSetName();

  FReply const someValue;

private:

	static TSharedRef< class FSlateStyleSet > Create();

	static TSharedPtr< class FSlateStyleSet > StyleInstance;
};
