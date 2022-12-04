// Copyright Epic Games, Inc. All Rights Reserved.

#include "MouseAnimationsCommands.h"

#define LOCTEXT_NAMESPACE "FMouseAnimationsModule"

void FMouseAnimationsCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "MouseAnimations", "Bring up MouseAnimations window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
