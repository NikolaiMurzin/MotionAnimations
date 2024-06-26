// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

#include "MotionAnimationsStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"
#include "Math/Range.h"



#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FMotionAnimationsStyle::StyleInstance = nullptr;

void FMotionAnimationsStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FMotionAnimationsStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FMotionAnimationsStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("MotionAnimationsStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FMotionAnimationsStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("MotionAnimationsStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("MotionAnimations")->GetBaseDir() / TEXT("Resources"));

	Style->Set("MotionAnimations.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	return Style;
}

void FMotionAnimationsStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FMotionAnimationsStyle::Get()
{
	return *StyleInstance;
}
