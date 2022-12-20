// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef MOUSEANIMATIONS_MotionHandlerMode_generated_h
#error "MotionHandlerMode.generated.h already included, missing '#pragma once' in MotionHandlerMode.h"
#endif
#define MOUSEANIMATIONS_MotionHandlerMode_generated_h

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_UnrealProjects_AllOfTheLights503_Plugins_MouseAnimations_Source_MouseAnimations_Private_MotionHandlerMode_h


#define FOREACH_ENUM_MODE(op) \
	op(Mode::X) \
	op(Mode::XInverted) \
	op(Mode::Y) \
	op(Mode::YInverted) 

enum class Mode : uint8;
template<> MOUSEANIMATIONS_API UEnum* StaticEnum<Mode>();

PRAGMA_ENABLE_DEPRECATION_WARNINGS
