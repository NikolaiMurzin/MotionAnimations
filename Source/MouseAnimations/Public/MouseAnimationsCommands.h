// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "MouseAnimationsStyle.h"

class FMouseAnimationsCommands : public TCommands<FMouseAnimationsCommands>
{
public:
	FMouseAnimationsCommands()
		: TCommands<FMouseAnimationsCommands>(TEXT("MouseAnimations"),
			  NSLOCTEXT("Contexts", "MouseAnimations", "MouseAnimations Plugin"), NAME_None,
			  FMouseAnimationsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenPluginWindow;
};
