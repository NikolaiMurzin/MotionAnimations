// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "Math/Range.h"


UENUM()
enum class Mode : uint8
{
	X,
	XInverted,
	Y,
	YInverted,
};
