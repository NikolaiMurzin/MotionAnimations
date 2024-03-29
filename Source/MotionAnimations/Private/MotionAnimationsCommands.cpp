// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

#include "MotionAnimationsCommands.h"

#define LOCTEXT_NAMESPACE "FMotionAnimationsModule"

void FMotionAnimationsCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "MotionAnimations", "Bring up MotionAnimations window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
