// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "MotionAnimationsStyle.h"

class FMotionAnimationsCommands : public TCommands<FMotionAnimationsCommands>
{
public:
	FMotionAnimationsCommands()
		: TCommands<FMotionAnimationsCommands>(TEXT("MotionAnimations"),
			  NSLOCTEXT("Contexts", "MotionAnimations", "MotionAnimations Plugin"), NAME_None,
			  FMotionAnimationsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenPluginWindow;
};
